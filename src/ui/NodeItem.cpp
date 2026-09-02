#include "NodeItem.h"
#include "FlowCanvas.h"
#include "Palette.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QtMath>
#include <QCoreApplication>

namespace autoflow {

// ---------------------------- NodeItem ----------------------------

NodeItem::NodeItem(FlowModel* model, const IInstruction* instr, const QString& nodeId,
                   QGraphicsItem* parent)
    : QGraphicsItem(parent), m_model(model), m_instr(instr), m_nodeId(nodeId) {
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setZValue(10);

    if (m_instr && m_instr->meta().hasInput)
        m_input = new PortItem(this, true, "", this);
    if (m_instr)
        for (auto& p : m_instr->meta().outPorts)
            m_outputs.push_back(new PortItem(this, false, QString::fromStdString(p), this));

    layoutPorts();
}

FlowNode* NodeItem::flowNode() const {
    return m_model ? m_model->nodeById(m_nodeId.toStdString()) : nullptr;
}

bool NodeItem::nodeEnabled() const {
    FlowNode* n = flowNode();
    return n ? n->enabled : true;
}

qreal NodeItem::nodeHeight() const {
    qreal headerH = 20;              // 分类标签 + ID 行
    qreal titleH  = 26;              // 标题
    qreal bodyH   = 8 + 2 * 15 + 8;  // 摘要（最多两行等宽读数）
    qreal portsH  = m_outputs.size() > 1 ? (qreal)m_outputs.size() * 22 : 20;
    (void)portsH;
    return headerH + titleH + bodyH;
}

QRectF NodeItem::boundingRect() const {
    // 留出阴影（下 4.5px）与选中/运行光晕（±5.5px）的绘制空间，避免被裁剪成硬边
    return QRectF(-6, -6, nodeWidth() + 12, nodeHeight() + 12);
}

QColor NodeItem::categoryColor(const std::string& category) {
    // 分类色：shadcn 协调的 Tailwind 柔和色阶（dark 用 400 阶、light 用 600 阶）
    const bool dark = ThemeManager::instance().effectiveDark();
    if (category == Category::Image)  return dark ? QColor("#A78BFA") : QColor("#7C3AED");  // violet
    if (category == Category::Input)  return dark ? QColor("#60A5FA") : QColor("#2563EB");  // blue
    if (category == Category::Data)   return dark ? QColor("#34D399") : QColor("#059669");  // emerald
    if (category == Category::Flow)   return dark ? QColor("#FBBF24") : QColor("#D97706");  // amber
    if (category == Category::AI)     return dark ? QColor("#E879F9") : QColor("#C026D3");  // fuchsia
    if (category == Category::Window) return dark ? QColor("#22D3EE") : QColor("#0891B2");  // cyan
    if (category == Category::File)   return dark ? QColor("#FB923C") : QColor("#EA580C");  // orange
    if (category == Category::System) return dark ? QColor("#A1A1AA") : QColor("#71717A");  // zinc
    return dark ? QColor("#A1A1AA") : QColor("#71717A");  // 未知分类 → 中性锌灰
}

void NodeItem::layoutPorts() {
    qreal h = nodeHeight();
    if (m_input) m_input->setPos(0, h / 2);
    if (m_outputs.size() == 1) {
        m_outputs[0]->setPos(nodeWidth(), h / 2);
    } else {
        qreal step = h / (m_outputs.size() + 1);
        for (int i = 0; i < m_outputs.size(); ++i)
            m_outputs[i]->setPos(nodeWidth(), step * (i + 1));
    }
}

QPointF NodeItem::inputScenePos() const {
    return m_input ? m_input->scenePos() : mapToScene(QPointF(0, nodeHeight() / 2));
}

QPointF NodeItem::outputScenePos(const QString& label) const {
    for (auto* p : m_outputs) if (p->label() == label) return p->scenePos();
    return outputScenePos(0);
}

QPointF NodeItem::outputScenePos(int index) const {
    if (m_outputs.isEmpty()) return mapToScene(QPointF(nodeWidth(), nodeHeight() / 2));
    if (index < 0 || index >= m_outputs.size()) index = 0;
    return m_outputs[index]->scenePos();
}

QString NodeItem::summaryText() const {
    if (!m_instr) return "";
    QStringList lines;
    FlowNode* n = flowNode();
    for (auto& p : m_instr->meta().params) {
        if (lines.size() >= 2) break;
        if (p.type == "textarea") continue;
        QString val = QString::fromStdString(p.def);
        if (n && n->params.contains(p.key)) {
            const json& v = n->params[p.key];
            if (v.is_string()) val = QString::fromStdString(v.get<std::string>());
            else if (v.is_boolean()) val = v.get<bool>()
                ? QCoreApplication::translate("NodeItem", "是")
                : QCoreApplication::translate("NodeItem", "否");
            else val = QString::fromStdString(v.dump());
        }
        if (val.size() > 14) val = val.left(13) + "…";
        lines << trInstr(p.label.c_str()) + ": " + val;
    }
    return lines.join("\n");
}

void NodeItem::refreshSummary() { update(); }

void NodeItem::setRunning(bool on) { m_running = on; update(); }

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    // 禁用节点：整体降透明度，直观表示"不执行"
    const bool disabled = !nodeEnabled();
    if (disabled) painter->setOpacity(0.45);

    qreal w = nodeWidth(), h = nodeHeight();
    QRectF body(0, 0, w, h);

    bool dark = false;
    FlowCanvasScene* sc = dynamic_cast<FlowCanvasScene*>(scene());
    if (sc) dark = sc->isDark();

    QColor cat = m_instr ? categoryColor(m_instr->meta().category) : Palette::textMute(dark);
    QString catName = m_instr ? trInstr(m_instr->meta().category.c_str()) : QString();
    QString title = m_instr ? trInstr(m_instr->meta().name.c_str()) : m_nodeId;

    const qreal radius = 12.0;  // 节点圆角 12px

    // 阴影分层：向下柔和投影，逐层下移并均分 alpha，叠加处更浓、边缘更淡
    if (!isSelected()) {
        painter->setPen(Qt::NoPen);
        const int shadowLayers = 3;
        const qreal shadowBase = Palette::nodeShadow(dark).alphaF();
        for (int i = 0; i < shadowLayers; ++i) {
            qreal off = 1.5 * (i + 1);                       // 1.5 / 3.0 / 4.5 px
            QColor sh = Palette::nodeShadow(dark);
            sh.setAlphaF(shadowBase / shadowLayers);         // 单层 alpha，叠加处更浓
            painter->setBrush(sh);
            painter->drawRoundedRect(body.translated(0, off), radius, radius);
        }
    }

    // 面板：纯色 + 描边，12px 圆角；选中态用 accentSubtle 底 + accent 边框
    painter->setBrush(isSelected() ? Palette::nodeSelectedBg(dark) : Palette::nodeBg(dark));
    painter->setPen(QPen(isSelected() ? Palette::nodeSelectedBorder(dark) : Palette::nodeBorder(dark),
                         isSelected() ? 1.5 : 1));
    painter->drawRoundedRect(body, radius, radius);

    // 选中：accent 外发光（多层圆角描边向外衰减）
    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        QColor accentC = Palette::accent(dark);
        const int glowLayers = 3;
        for (int i = 0; i < glowLayers; ++i) {
            qreal off = 1.5 * (i + 1);                       // 1.5 / 3.0 / 4.5 px
            QColor g = accentC;
            g.setAlphaF(0.20 * (glowLayers - i) / glowLayers); // 越靠外越淡
            painter->setPen(QPen(g, 2.0));
            painter->drawRoundedRect(body.adjusted(-off, -off, off, off), radius + off, radius + off);
        }
    }

    // 分类标签 pill（左上）
    QFont pillFont = ThemeManager::smileySansFont(); pillFont.setPixelSize(9);
    painter->setFont(pillFont);
    QFontMetrics pillFm(pillFont);
    int pillW = pillFm.horizontalAdvance(catName) + 12;
    QRectF pill(14, 6, pillW, 16);
    QColor pillBg = cat; pillBg.setAlphaF(0.22f);
    painter->setPen(Qt::NoPen);
    painter->setBrush(pillBg);
    painter->drawRoundedRect(pill, 5, 5);
    painter->setPen(cat);
    painter->drawText(pill, Qt::AlignCenter, catName);

    // 节点 ID（弱化，用全局字体避免中文字 fallback 粗）
    QFont idFont = ThemeManager::smileySansFont(); idFont.setPixelSize(9);
    painter->setFont(idFont);
    painter->setPen(Palette::textMute(dark));
    painter->drawText(QRectF(pill.right() + 8, 6, w - 14 - (pill.right() + 8), 16),
                      Qt::AlignRight | Qt::AlignVCenter, m_nodeId);

    // 标题
    painter->setPen(Palette::nodeTitle(dark));
    QFont tFont = ThemeManager::smileySansFont(); tFont.setPixelSize(13);
    painter->setFont(tFont);
    painter->drawText(QRectF(14, 24, w - 28, 22), Qt::AlignVCenter | Qt::AlignLeft, title);

    // 摘要（用全局字体，避免 Consolas 缺中文字 fallback 到系统默认导致粗细不一致）
    painter->setPen(Palette::nodeText(dark));
    QFont sumFont = ThemeManager::smileySansFont(); sumFont.setPixelSize(10);
    painter->setFont(sumFont);
    painter->drawText(QRectF(14, 48, w - 28, h - 54), Qt::AlignLeft | Qt::AlignTop, summaryText());

    // 端口标签
    painter->setPen(Palette::nodeText(dark));
    QFont pf = ThemeManager::smileySansFont(); pf.setPixelSize(10);
    painter->setFont(pf);
    if (m_input) painter->drawText(QRectF(8, h / 2 - 14, 40, 16), Qt::AlignLeft | Qt::AlignVCenter,
                                   QCoreApplication::translate("NodeItem", "入"));
    for (auto* p : m_outputs) {
        QPointF pp = p->pos();
        // 出端口标签本地化（"next" → 翻译；"真/假" 是格式串，translate 返回源串不变）
        QString lbl = trInstr(p->label().toUtf8().constData());
        painter->drawText(QRectF(pp.x() - 60, pp.y() - 8, 54, 16),
                          Qt::AlignRight | Qt::AlignVCenter, lbl);
    }

    // 运行结果状态：失败红描边 + 左上红点；完成绿勾点
    if (FlowNode* fn = flowNode()) {
        const std::string& rs = fn->runState;
        if (rs == "error") {
            painter->setPen(QPen(Palette::stop(dark), 2.0));
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(body.adjusted(-1.5, -1.5, 1.5, 1.5), radius + 1.5, radius + 1.5);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Palette::stop(dark));
            painter->drawEllipse(QPointF(6, 6), 5, 5);
        } else if (rs == "ok") {
            painter->setPen(Qt::NoPen);
            painter->setBrush(Palette::run(dark));
            painter->drawEllipse(QPointF(6, 6), 5, 5);
            painter->setPen(QPen(dark ? Palette::bg(dark) : Qt::white, 1.6));
            painter->setBrush(Qt::NoBrush);
            QPainterPath chk;
            chk.moveTo(3.6, 6.2); chk.lineTo(5.4, 8.0); chk.lineTo(8.6, 4.2);
            painter->drawPath(chk);
        }
    }

    // 运行中：蓝色脉冲描边（监控高亮）
    if (m_running && sc) {
        qreal t = sc->runProgress();
        const qreal kPi = 3.14159265358979323846;
        qreal k = 0.5 + 0.5 * qSin(t * 2.0 * kPi);
        QColor glow = Palette::accent(dark);
        glow.setAlphaF(float(0.10 + 0.45 * k));
        painter->setPen(Qt::NoPen);
        painter->setBrush(glow);
        painter->drawRoundedRect(body.adjusted(-3, -3, 3, 3), radius + 3, radius + 3);
        painter->setPen(QPen(Palette::accent(dark), 1.8));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(body.adjusted(1.5, 1.5, -1.5, -1.5), radius - 1.5, radius - 1.5);
    }
}

void NodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* e) {
    FlowCanvasScene* sc = dynamic_cast<FlowCanvasScene*>(scene());
    QMenu menu;
    QAction* runAct = menu.addAction(QCoreApplication::translate("NodeItem", "从此运行"));
    QAction* dupAct = menu.addAction(QCoreApplication::translate("NodeItem", "复制节点"));
    const bool en = nodeEnabled();
    QAction* disAct = menu.addAction(en ? QCoreApplication::translate("NodeItem", "禁用")
                                        : QCoreApplication::translate("NodeItem", "启用"));
    menu.addSeparator();
    QAction* delAct = menu.addAction(QCoreApplication::translate("NodeItem", "删除"));
    QAction* chosen = menu.exec(e->screenPos());
    if (!chosen || !sc) { e->accept(); return; }
    if (chosen == runAct) emit sc->requestRunFrom(m_nodeId);
    else if (chosen == dupAct) sc->duplicateNode(m_nodeId);
    else if (chosen == disAct) sc->setNodeEnabled(m_nodeId, !en);
    else if (chosen == delAct) sc->deleteNode(m_nodeId);
    e->accept();
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) {
        if (FlowNode* n = flowNode()) { n->x = pos().x(); n->y = pos().y(); }
        if (m_model) m_model->setDirty(true);
        if (auto* s = dynamic_cast<FlowCanvasScene*>(scene()))
            s->onNodeMoved(this);
    }
    return QGraphicsItem::itemChange(change, value);
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_moveBeforePos = pos();
        if (auto* s = dynamic_cast<FlowCanvasScene*>(scene()))
            m_moveBeforeSnapshot = s->snapshotJson();
        m_moveCaptured = true;
    }
    QGraphicsItem::mousePressEvent(e);
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsItem::mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton && m_moveCaptured) {
        m_moveCaptured = false;
        if (pos() != m_moveBeforePos) {
            if (auto* s = dynamic_cast<FlowCanvasScene*>(scene()))
                s->pushUndoSnapshot(m_moveBeforeSnapshot);
        }
        m_moveBeforeSnapshot = json();
    }
}

// ---------------------------- PortItem ----------------------------

PortItem::PortItem(NodeItem* owner, bool isInput, const QString& label, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_owner(owner), m_input(isInput), m_label(label) {
    setAcceptHoverEvents(true);
    setZValue(20);
}

QRectF PortItem::boundingRect() const { return QRectF(-9, -9, 18, 18); }

void PortItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    bool dark = false;
    if (auto* s = dynamic_cast<FlowCanvasScene*>(scene())) dark = s->isDark();

    // 端口区分：入端口=蓝色实心圆，出端口=琥珀色空心环（拖拽起点）
    QColor ring;
    if (m_connectHint == HintValid) ring = Palette::run(dark);           // 可落点：绿
    else if (m_connectHint == HintInvalid) ring = Palette::textMute(dark); // 非法目标：灰
    else if (m_input) ring = Palette::accent(dark);                      // 入端口：蓝
    else ring = Palette::warn(dark);                                     // 出端口：琥珀
    if (m_hover && m_connectHint == HintNone) ring = ring.lighter(125);

    const qreal r = (m_hover || m_dragActive) ? 6.0 : 4.5;
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(ring, m_hover ? 2.0 : 1.4));
    if (m_input) {
        painter->setBrush(ring);
        painter->drawEllipse(QPointF(0, 0), r, r);
    } else {
        painter->setBrush(Palette::portFill(dark));
        painter->drawEllipse(QPointF(0, 0), r, r);
        painter->setPen(Qt::NoPen);
        painter->setBrush(ring);
        painter->drawEllipse(QPointF(0, 0), 2.2, 2.2);
    }

    // 拖线时：可落点绿色描边强调，鼠标下方端额外放大
    if (m_connectHint == HintValid) {
        painter->setPen(QPen(Palette::run(dark), (m_dragActive ? 2.6 : 1.8)));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QPointF(0, 0), r + 3, r + 3);
    }
}

void PortItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton && !m_input) {
        m_dragging = true;
        if (auto* s = dynamic_cast<FlowCanvasScene*>(scene()))
            s->beginConnection(this);
        e->accept();
    }
}

void PortItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    if (m_dragging) {
        if (auto* s = dynamic_cast<FlowCanvasScene*>(scene()))
            s->updateConnection(e->scenePos());
        e->accept();
    }
}

void PortItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    if (m_dragging) {
        m_dragging = false;
        if (auto* s = dynamic_cast<FlowCanvasScene*>(scene()))
            s->endConnection(e->scenePos());
        e->accept();
    }
}

void PortItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e) {
    m_hover = true;
    setCursor(m_input ? Qt::PointingHandCursor : Qt::CrossCursor);
    update();
    QGraphicsItem::hoverEnterEvent(e);
}

void PortItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e) {
    m_hover = false;
    unsetCursor();
    update();
    QGraphicsItem::hoverLeaveEvent(e);
}

} // namespace autoflow
