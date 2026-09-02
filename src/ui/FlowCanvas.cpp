#include "FlowCanvas.h"
#include "Palette.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QCoreApplication>
#include <QApplication>
#include <QClipboard>
#include <QToolTip>

namespace autoflow {

namespace {
// 拖拽指令时的幽灵预览卡片（半透明 + 虚线描边 + 分类色侧条）
class GhostItem : public QGraphicsItem {
public:
    GhostItem(const QString& name, const QColor& cat, bool dark) : m_name(name), m_cat(cat), m_dark(dark) {
        setZValue(50);
    }
    QRectF boundingRect() const override { return QRectF(0, 0, 200, 48); }
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override {
        p->setRenderHint(QPainter::Antialiasing, true);
        QColor cardBg = Palette::surface3(m_dark); cardBg.setAlpha(70);
        p->setBrush(cardBg);
        QColor dashBorder = Palette::borderStrong(m_dark); dashBorder.setAlpha(150);
        p->setPen(QPen(dashBorder, 1, Qt::DashLine));
        p->drawRoundedRect(QRectF(0, 0, 200, 48), 6, 6);
        QColor bar = m_cat; bar.setAlphaF(0.5f);
        p->setPen(Qt::NoPen);
        p->setBrush(bar);
        p->drawRoundedRect(QRectF(6, 6, 4, 36), 2, 2);
        QFont f = ThemeManager::smileySansFont(); f.setPixelSize(12);
        p->setFont(f);
        QColor ghostText = Palette::text(m_dark); ghostText.setAlpha(200);
        p->setPen(ghostText);
        p->drawText(QRectF(20, 0, 174, 48), Qt::AlignVCenter | Qt::AlignLeft, m_name);
    }
private:
    QString m_name;
    QColor m_cat;
    bool m_dark = false;
};
} // namespace

// ---------------------------- FlowCanvasScene ----------------------------

FlowCanvasScene::FlowCanvasScene(FlowModel* model, QObject* parent)
    : QGraphicsScene(parent), m_model(model) {
    setSceneRect(0, 0, 4000, 3000);
    connect(this, &QGraphicsScene::selectionChanged, this, &FlowCanvasScene::onSelectionChanged);
    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(50);
    connect(m_pulseTimer, &QTimer::timeout, this, &FlowCanvasScene::onPulseTick);
}

void FlowCanvasScene::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsScene::drawBackground(painter, rect);

    // 监控仪表盘画布：扁平深色底色（无渐变）
    painter->fillRect(rect, Palette::canvasBg(m_dark));

    // 点阵图纸：细点 + 每 100px 大点
    const int step = 20, major = 100;
    painter->setPen(Qt::NoPen);

    painter->setBrush(Palette::canvasGridMinor(m_dark));
    qreal left = (int)rect.left() - ((int)rect.left() % step);
    qreal top = (int)rect.top() - ((int)rect.top() % step);
    for (qreal x = left; x < rect.right(); x += step)
        for (qreal y = top; y < rect.bottom(); y += step)
            painter->drawEllipse(QPointF(x, y), 1.0, 1.0);

    painter->setBrush(Palette::canvasGridMajor(m_dark));
    qreal l2 = (int)rect.left() - ((int)rect.left() % major);
    qreal t2 = (int)rect.top() - ((int)rect.top() % major);
    for (qreal x = l2; x < rect.right(); x += major)
        for (qreal y = t2; y < rect.bottom(); y += major)
            painter->drawEllipse(QPointF(x, y), 1.6, 1.6);
}

void FlowCanvasScene::drawForeground(QPainter* painter, const QRectF& rect) {
    QGraphicsScene::drawForeground(painter, rect);

    if (views().isEmpty()) return;

    // 抗缩放：重置为恒等变换，用 viewport 坐标（屏幕像素）绘制提示，
    // 让提示文字保持固定大小，不随画布缩放而放大/缩小。
    painter->save();
    painter->setWorldTransform(QTransform());
    const QRect vp = views().first()->viewport()->rect();

    // 操作提示：框选需按住 Shift，含常用快捷键（右下角弱化显示）
    {
        const QPointF bl(16, vp.height() - 10.0);
        painter->setRenderHint(QPainter::Antialiasing, true);
        QFont hf = ThemeManager::smileySansFont(); hf.setPixelSize(10);
        painter->setFont(hf);
        painter->setPen(Palette::textMute(m_dark));
        painter->drawText(bl,
            QCoreApplication::translate("FlowCanvas",
                "Shift+拖拽框选 · Ctrl+C/V 复制粘贴 · Ctrl+A 全选 · Ctrl+Z/Y 撤销重做"));
    }

    // 空画布引导：没有内容节点时，在可见区域中央提示
    int contentNodes = 0;
    for (auto& n : m_model->nodes) if (n.instr != "start") ++contentNodes;
    if (contentNodes > 0) { painter->restore(); return; }

    const QPointF center(vp.width() / 2.0, vp.height() / 2.0);

    painter->setRenderHint(QPainter::Antialiasing, true);
    QRectF box(center.x() - 190, center.y() - 42, 380, 84);
    painter->setPen(QPen(Palette::canvasGridMajor(m_dark), 1.2, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(box, 8, 8);

    QFont f = ThemeManager::smileySansFont(); f.setPixelSize(14);
    painter->setFont(f);
    painter->setPen(Palette::textMute(m_dark));
    painter->drawText(box.adjusted(0, -12, 0, -12), Qt::AlignCenter,
                      QCoreApplication::translate("FlowCanvas", "从左侧指令面板拖拽指令到画布"));
    QFont f2 = ThemeManager::smileySansFont(); f2.setPixelSize(11);
    painter->setFont(f2);
    painter->drawText(box.adjusted(0, 12, 0, 12), Qt::AlignCenter,
                      QCoreApplication::translate("FlowCanvas", "连接节点即可编排自动化流程"));

    painter->restore();
}

void FlowCanvasScene::rebuildFromModel() {
    clear();
    m_items.clear();
    m_edges.clear();
    m_tempFrom = nullptr;
    m_tempEdge = nullptr;
    m_ghost = nullptr;

    for (auto& n : m_model->nodes) {
        const IInstruction* instr = InstructionRegistry::instance().get(n.instr);
        NodeItem* item = new NodeItem(m_model, instr, QString::fromStdString(n.id));
        addItem(item);
        item->setPos(n.x, n.y);
        m_items.insert(QString::fromStdString(n.id), item);
    }
    rebuildEdges();
}

void FlowCanvasScene::loadFromModel() {
    rebuildFromModel();
    // 新文档：清空撤销/重做历史
    m_undoStack.clear();
    m_redoStack.clear();
    endParamSession();
    emit undoAvailable(false);
    emit redoAvailable(false);
}

void FlowCanvasScene::endParamSession() {
    m_paramEditActive = false;
    m_paramEditPushed = false;
    m_paramEditNode.clear();
    m_paramEditBefore = json();
}

void FlowCanvasScene::recordUndoPoint() {
    endParamSession();
    m_undoStack.push_back(m_model->toJson());
    if (m_undoStack.size() > 100) m_undoStack.removeFirst();
    m_redoStack.clear();
    emit undoAvailable(true);
    emit redoAvailable(false);
}

void FlowCanvasScene::pushUndoSnapshot(const json& before) {
    endParamSession();
    if (before.is_null()) return;
    m_undoStack.push_back(before);
    if (m_undoStack.size() > 100) m_undoStack.removeFirst();
    m_redoStack.clear();
    emit undoAvailable(true);
    emit redoAvailable(false);
}

void FlowCanvasScene::restoreSnapshot(const json& j) {
    if (!m_model || j.is_null()) return;
    m_model->fromJson(j);
    m_model->setDirty(true);
    rebuildFromModel();
    clearConnectHints();
    clearSelection();
    emit nodeSelected(QString());
    emit modelChanged();
}

void FlowCanvasScene::undo() {
    if (m_undoStack.isEmpty()) return;
    json before = m_undoStack.takeLast();
    m_redoStack.push_back(m_model->toJson());
    restoreSnapshot(before);
    emit undoAvailable(!m_undoStack.isEmpty());
    emit redoAvailable(!m_redoStack.isEmpty());
}

void FlowCanvasScene::redo() {
    if (m_redoStack.isEmpty()) return;
    json after = m_redoStack.takeLast();
    m_undoStack.push_back(m_model->toJson());
    restoreSnapshot(after);
    emit undoAvailable(!m_undoStack.isEmpty());
    emit redoAvailable(!m_redoStack.isEmpty());
}

void FlowCanvasScene::rebuildEdges() {
    for (auto* e : m_edges) { removeItem(e); delete e; }
    m_edges.clear();
    for (auto& e : m_model->edges) {
        NodeItem* from = m_items.value(QString::fromStdString(e.from));
        NodeItem* to = m_items.value(QString::fromStdString(e.to));
        if (from && to) {
            EdgeItem* ei = new EdgeItem(from, to, QString::fromStdString(e.label));
            addItem(ei);
            m_edges.append(ei);
        }
    }
}

void FlowCanvasScene::autoChain(NodeItem* item) {
    if (!item || !item->instr()) return;
    if (!item->instr()->meta().hasInput) return;   // start 节点不连入

    // 找唯一的“线性”尾节点（无出边且输出端口为 next）
    QString tail;
    int tailCount = 0;
    for (auto& n : m_model->nodes) {
        if (n.id == item->nodeId().toStdString()) continue;
        if (!m_model->outEdges(n.id).empty()) continue;
        const IInstruction* ii = InstructionRegistry::instance().get(n.instr);
        bool linear = ii && ii->meta().outPorts.size() == 1 && ii->meta().outPorts[0] == "next";
        if (linear) { tail = QString::fromStdString(n.id); ++tailCount; }
    }
    if (tailCount != 1 || tail.isEmpty()) return;

    FlowEdge e;
    e.from = tail.toStdString();
    e.to = item->nodeId().toStdString();
    e.label = "next";
    m_model->addEdge(e);

    NodeItem* from = m_items.value(tail);
    EdgeItem* ei = new EdgeItem(from, item, "next");
    addItem(ei);
    m_edges.append(ei);
    emit modelChanged();
}

void FlowCanvasScene::addNode(const std::string& instrId, const QPointF& pos) {
    const IInstruction* instr = InstructionRegistry::instance().get(instrId);
    if (!instr) return;

    recordUndoPoint();

    FlowNode n;
    n.id = m_model->newId();
    n.instr = instrId;
    n.x = pos.x();
    n.y = pos.y();
    m_model->addNode(n);

    NodeItem* item = new NodeItem(m_model, instr, QString::fromStdString(n.id));
    addItem(item);
    item->setPos(pos);
    m_items.insert(QString::fromStdString(n.id), item);

    autoChain(item);
    emit modelChanged();
}

void FlowCanvasScene::deleteSelection() {
    QList<QGraphicsItem*> sel = selectedItems();
    if (sel.isEmpty()) return;
    recordUndoPoint();

    // 先收集待删节点 id 集合（避免删除节点后 EdgeItem 的 from/to 变成悬空指针）
    QSet<QString> removed;
    for (auto* it : sel) {
        if (auto* ni = dynamic_cast<NodeItem*>(it))
            removed.insert(ni->nodeId());
    }

    // 先删连线（端点属于待删集合，或被选中的边），再删节点
    for (int i = m_edges.size() - 1; i >= 0; --i) {
        EdgeItem* e = m_edges[i];
        if (removed.contains(e->fromNode()->nodeId()) || removed.contains(e->toNode()->nodeId())
            || sel.contains(e)) {
            m_model->removeEdge(e->fromNode()->nodeId().toStdString(), e->label().toStdString());
            m_edges.removeAt(i);
            removeItem(e);
            delete e;
        }
    }

    // 最后删节点
    for (auto* it : sel) {
        if (auto* ni = dynamic_cast<NodeItem*>(it)) {
            m_model->removeNode(ni->nodeId().toStdString());
            m_items.remove(ni->nodeId());
            removeItem(ni);
            delete ni;
        }
    }

    emit modelChanged();
    emit nodeSelected(QString());
}

void FlowCanvasScene::deleteNode(const QString& id) {
    if (!m_items.contains(id)) return;
    recordUndoPoint();
    NodeItem* ni = m_items.value(id);
    m_model->removeNode(id.toStdString());
    m_items.remove(id);
    // 删除相关连线
    for (int i = m_edges.size() - 1; i >= 0; --i) {
        EdgeItem* e = m_edges[i];
        if (e->fromNode()->nodeId() == id || e->toNode()->nodeId() == id) {
            m_model->removeEdge(e->fromNode()->nodeId().toStdString(), e->label().toStdString());
            m_edges.removeAt(i);
            removeItem(e);
            delete e;
        }
    }
    removeItem(ni);
    delete ni;
    emit modelChanged();
    emit nodeSelected(QString());
}

void FlowCanvasScene::duplicateNode(const QString& id) {
    NodeItem* src = m_items.value(id);
    FlowNode* sn = m_model->nodeById(id.toStdString());
    if (!src || !sn) return;
    recordUndoPoint();
    FlowNode n;
    n.id = m_model->newId();
    n.instr = sn->instr;
    n.params = sn->params;
    n.x = sn->x + 32;
    n.y = sn->y + 32;
    n.onError = sn->onError;
    n.retry = sn->retry;
    n.comment = sn->comment;
    n.enabled = sn->enabled;
    m_model->addNode(n);
    NodeItem* item = new NodeItem(m_model, src->instr(), QString::fromStdString(n.id));
    addItem(item);
    item->setPos(n.x, n.y);
    m_items.insert(QString::fromStdString(n.id), item);
    emit modelChanged();
}

void FlowCanvasScene::setNodeEnabled(const QString& id, bool on) {
    if (FlowNode* n = m_model->nodeById(id.toStdString())) {
        recordUndoPoint();
        n->enabled = on;
        m_model->setDirty(true);
        if (NodeItem* ni = m_items.value(id)) ni->update();
        emit modelChanged();
    }
}

void FlowCanvasScene::showGhost(const QString& instrId, const QPointF& scenePos) {
    hideGhost();
    const IInstruction* instr = InstructionRegistry::instance().get(instrId.toStdString());
    if (!instr) return;
    QColor cat = NodeItem::categoryColor(instr->meta().category);
    m_ghost = new GhostItem(trInstr(instr->meta().name.c_str()), cat, m_dark);
    addItem(m_ghost);
    m_ghost->setPos(scenePos - QPointF(100, 24));
}

void FlowCanvasScene::moveGhost(const QPointF& scenePos) {
    if (m_ghost) m_ghost->setPos(scenePos - QPointF(100, 24));
}

void FlowCanvasScene::hideGhost() {
    if (m_ghost) { removeItem(m_ghost); delete m_ghost; m_ghost = nullptr; }
}

void FlowCanvasScene::refreshNodeSummary(const QString& id) {
    if (NodeItem* ni = m_items.value(id)) ni->refreshSummary();

    // 参数写回（ParamPanel 直接改模型字段后触发）：同一选中节点连续编辑合并为一个撤销点。
    // “编辑前”快照在选中节点时（onSelectionChanged）捕获。
    if (m_paramEditActive && m_paramEditNode == id && !m_paramEditPushed) {
        m_undoStack.push_back(m_paramEditBefore);
        if (m_undoStack.size() > 100) m_undoStack.removeFirst();
        m_redoStack.clear();
        m_paramEditPushed = true;
        emit undoAvailable(true);
        emit redoAvailable(false);
    }
    m_model->setDirty(true);
}

void FlowCanvasScene::highlightNode(const QString& id) {
    m_runningId = id;
    m_runProgress = 0;
    for (auto* it : m_items) it->setRunning(it->nodeId() == id);
    m_pulseTimer->start();
}

void FlowCanvasScene::clearHighlight() {
    m_runningId.clear();
    for (auto* it : m_items) it->setRunning(false);
    m_pulseTimer->stop();
}

void FlowCanvasScene::markNodeState(const QString& id, const QString& state) {
    if (FlowNode* n = m_model->nodeById(id.toStdString()))
        n->runState = state.toStdString();

    if (state == "running") {
        highlightNode(id);
        return;
    }
    // 结束该节点的运行脉冲（若仍在跑）
    if (m_runningId == id) {
        m_runningId.clear();
        m_pulseTimer->stop();
        if (NodeItem* ni = m_items.value(id)) ni->setRunning(false);
    }
    if (NodeItem* ni = m_items.value(id)) ni->update();
}

void FlowCanvasScene::clearRunStates() {
    for (auto& n : m_model->nodes) n.runState = "none";
    for (auto* ni : m_items) ni->update();
}

void FlowCanvasScene::onPulseTick() {
    m_runProgress += 0.05;
    if (m_runProgress > 1.0) m_runProgress -= 1.0;
    if (NodeItem* running = m_items.value(m_runningId)) running->update();
    for (auto* e : m_edges) {
        if (e->fromNode()->nodeId() == m_runningId || e->toNode()->nodeId() == m_runningId)
            e->update();
    }
}

void FlowCanvasScene::beginConnection(PortItem* from) {
    m_tempFrom = from;
    m_tempEdge = new QGraphicsPathItem();
    m_tempEdge->setPen(QPen(Palette::accent(m_dark), 2, Qt::DashLine));
    m_tempEdge->setZValue(30);
    addItem(m_tempEdge);

    // 拖线期间标记端口可用性：可落点的入端口绿色，其余灰色
    for (auto* ni : m_items) {
        if (PortItem* ip = ni->inputPort())
            ip->setConnectHint(ip->owner() == from->owner() ? PortItem::HintInvalid : PortItem::HintValid);
        for (auto* op : ni->outputPorts())
            op->setConnectHint(PortItem::HintInvalid);
    }

    updateConnection(from->scenePos());
}

PortItem* FlowCanvasScene::portAt(const QPointF& scenePos) {
    QList<QGraphicsItem*> under = items(QRectF(scenePos.x() - 10, scenePos.y() - 10, 20, 20));
    for (auto* it : under) {
        if (auto* p = dynamic_cast<PortItem*>(it)) return p;
    }
    return nullptr;
}

void FlowCanvasScene::updateConnection(const QPointF& scenePos) {
    if (!m_tempEdge || !m_tempFrom) return;
    QPointF s = m_tempFrom->scenePos();
    QPointF e = scenePos;
    qreal bend = qBound(40.0, qAbs(e.x() - s.x()) * 0.5, 180.0);
    QPainterPath p(s);
    p.cubicTo(QPointF(s.x() + bend, s.y()), QPointF(e.x() - bend, e.y()), e);
    m_tempEdge->setPath(p);

    // 实时强调鼠标下方的入端口
    PortItem* under = portAt(scenePos);
    for (auto* ni : m_items) {
        if (PortItem* ip = ni->inputPort())
            ip->setDragActive(ip == under && ip->connectHint() == PortItem::HintValid);
    }
}

void FlowCanvasScene::clearConnectHints() {
    for (auto* ni : m_items) {
        if (PortItem* ip = ni->inputPort()) {
            ip->setConnectHint(PortItem::HintNone);
            ip->setDragActive(false);
        }
        for (auto* op : ni->outputPorts()) {
            op->setConnectHint(PortItem::HintNone);
            op->setDragActive(false);
        }
    }
}

void FlowCanvasScene::endConnection(const QPointF& scenePos) {
    if (!m_tempFrom) return;
    PortItem* target = nullptr;
    QList<QGraphicsItem*> under = items(QRectF(scenePos.x() - 8, scenePos.y() - 8, 16, 16));
    for (auto* it : under) {
        if (auto* p = dynamic_cast<PortItem*>(it)) {
            if (p->isInput() && p->owner() != m_tempFrom->owner()) { target = p; break; }
        }
    }

    clearConnectHints();
    if (target) {
        connectPorts(m_tempFrom, target);
    } else {
        // 拖到空白处松手：一次性提示
        QString msg = QCoreApplication::translate("FlowCanvas",
            "未连接到端口：请把连线拖到左侧入端口上释放");
        emit statusMessage(msg);
        if (!views().isEmpty()) {
            QPoint gp = views().first()->mapToGlobal(views().first()->mapFromScene(scenePos));
            QToolTip::showText(gp, msg);
        }
    }

    if (m_tempEdge) { removeItem(m_tempEdge); delete m_tempEdge; m_tempEdge = nullptr; }
    m_tempFrom = nullptr;
}

void FlowCanvasScene::connectPorts(PortItem* from, PortItem* to) {
    QString fromNode = from->owner()->nodeId();
    QString toNode = to->owner()->nodeId();
    QString label = from->label();
    if (fromNode == toNode) return;

    recordUndoPoint();
    removeEdgeItem(fromNode, label);
    m_model->removeEdge(fromNode.toStdString(), label.toStdString());

    FlowEdge e;
    e.from = fromNode.toStdString();
    e.to = toNode.toStdString();
    e.label = label.toStdString();
    m_model->addEdge(e);

    EdgeItem* ei = new EdgeItem(from->owner(), to->owner(), label);
    addItem(ei);
    m_edges.append(ei);
    emit modelChanged();
}

void FlowCanvasScene::removeEdgeItem(const QString& from, const QString& label) {
    for (int i = m_edges.size() - 1; i >= 0; --i) {
        if (m_edges[i]->fromNode()->nodeId() == from && m_edges[i]->label() == label) {
            removeItem(m_edges[i]);
            delete m_edges[i];
            m_edges.removeAt(i);
        }
    }
}

void FlowCanvasScene::onNodeMoved(NodeItem* node) {
    for (auto* e : m_edges) {
        if (e->fromNode() == node || e->toNode() == node) e->refresh();
    }
}

void FlowCanvasScene::onSelectionChanged() {
    QString id;
    for (auto* it : selectedItems()) {
        if (auto* ni = dynamic_cast<NodeItem*>(it)) { id = ni->nodeId(); break; }
    }

    // 选中节点变化：开启/切换参数编辑合并会话，并捕获“编辑前”快照
    if (id != m_paramEditNode || !m_paramEditActive) {
        endParamSession();
        if (!id.isEmpty()) {
            m_paramEditNode = id;
            m_paramEditBefore = m_model->toJson();
            m_paramEditActive = true;
            m_paramEditPushed = false;
        }
    }
    emit nodeSelected(id);
}

void FlowCanvasScene::copySelection() {
    QSet<QString> ids;
    json jnodes = json::array();
    for (auto* it : selectedItems()) {
        if (auto* ni = dynamic_cast<NodeItem*>(it)) {
            FlowNode* n = m_model->nodeById(ni->nodeId().toStdString());
            if (!n) continue;
            json nj;
            nj["id"] = n->id;
            nj["instr"] = n->instr;
            nj["params"] = n->params;
            nj["x"] = n->x;
            nj["y"] = n->y;
            nj["onError"] = n->onError;
            nj["retry"] = n->retry;
            nj["comment"] = n->comment;
            nj["enabled"] = n->enabled;
            jnodes.push_back(nj);
            ids.insert(QString::fromStdString(n->id));
        }
    }
    if (jnodes.empty()) return;

    json jedges = json::array();
    for (auto& e : m_model->edges) {
        if (ids.contains(QString::fromStdString(e.from)) &&
            ids.contains(QString::fromStdString(e.to)))
            jedges.push_back({ {"from", e.from}, {"to", e.to}, {"label", e.label} });
    }
    json j;
    j["nodes"] = jnodes;
    j["edges"] = jedges;
    QApplication::clipboard()->setText(QString::fromStdString(j.dump()));
    emit statusMessage(QCoreApplication::translate("FlowCanvas", "已复制 %1 个节点").arg(jnodes.size()));
}

void FlowCanvasScene::pasteClipboard() {
    QString text = QApplication::clipboard()->text();
    json j;
    try { j = json::parse(text.toStdString()); }
    catch (...) { return; }
    if (!j.is_object() || !j.contains("nodes") || !j["nodes"].is_array()) return;
    if (j["nodes"].empty()) return;

    recordUndoPoint();
    QHash<QString, QString> idMap;
    clearSelection();

    for (auto& nj : j["nodes"]) {
        const IInstruction* instr = InstructionRegistry::instance().get(nj.value("instr", ""));
        FlowNode n;
        n.id = m_model->newId();
        n.instr = nj.value("instr", "");
        n.params = nj.value("params", json::object());
        n.x = nj.value("x", 0.0) + 24;
        n.y = nj.value("y", 0.0) + 24;
        n.onError = nj.value("onError", OnError::Abort);
        n.retry = nj.value("retry", 0);
        n.comment = nj.value("comment", "");
        n.enabled = nj.value("enabled", true);
        QString oldId = QString::fromStdString(nj.value("id", ""));
        m_model->addNode(n);
        idMap[oldId] = QString::fromStdString(n.id);

        NodeItem* item = new NodeItem(m_model, instr, QString::fromStdString(n.id));
        addItem(item);
        item->setPos(n.x, n.y);
        item->setSelected(true);
        m_items.insert(QString::fromStdString(n.id), item);
    }

    if (j.contains("edges") && j["edges"].is_array()) {
        for (auto& ej : j["edges"]) {
            QString fromOld = QString::fromStdString(ej.value("from", ""));
            QString toOld = QString::fromStdString(ej.value("to", ""));
            if (!idMap.contains(fromOld) || !idMap.contains(toOld)) continue;
            FlowEdge e;
            e.from = idMap[fromOld].toStdString();
            e.to = idMap[toOld].toStdString();
            e.label = ej.value("label", "next");
            m_model->addEdge(e);
        }
    }
    rebuildEdges();
    emit modelChanged();
}

void FlowCanvasScene::selectAllNodes() {
    clearSelection();
    for (auto* ni : m_items) ni->setSelected(true);
}

// ---------------------------- FlowCanvas ----------------------------

FlowCanvas::FlowCanvas(FlowModel* model, QWidget* parent) : QGraphicsView(parent) {
    m_scene = new FlowCanvasScene(model, this);
    setScene(m_scene);
    setObjectName("flowCanvas");
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);   // 左键拖空白处平移画布
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setAcceptDrops(true);
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);   // 最小区域重绘，避免拖拽残影
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);          // 隐藏滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void FlowCanvas::wheelEvent(QWheelEvent* e) {
    double factor = e->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    double current = transform().m11();
    if (current * factor > 4 || current * factor < 0.1) return;
    scale(factor, factor);
    emit zoomChanged(zoomLevel());
    e->accept();
}

// 左键拖空白处 = 平移（ScrollHandDrag）；Shift+左键拖空白处 = 框选
void FlowCanvas::mousePressEvent(QMouseEvent* e) {
    bool hasItem = (itemAt(e->pos()) != nullptr);
    if (e->button() == Qt::LeftButton && !hasItem && (e->modifiers() & Qt::ShiftModifier)) {
        setDragMode(QGraphicsView::RubberBandDrag);
        QGraphicsView::mousePressEvent(e);
    } else {
        QGraphicsView::mousePressEvent(e);
    }
}

void FlowCanvas::mouseReleaseEvent(QMouseEvent* e) {
    QGraphicsView::mouseReleaseEvent(e);
    setDragMode(QGraphicsView::ScrollHandDrag);   // 恢复平移
}

void FlowCanvas::keyPressEvent(QKeyEvent* e) {
    if (e->matches(QKeySequence::Undo)) {
        m_scene->undo();
        e->accept();
        return;
    }
    if (e->matches(QKeySequence::Redo)) {
        m_scene->redo();
        e->accept();
        return;
    }
    if (e->matches(QKeySequence::Copy)) {
        m_scene->copySelection();
        e->accept();
        return;
    }
    if (e->matches(QKeySequence::Paste)) {
        m_scene->pasteClipboard();
        e->accept();
        return;
    }
    if (e->matches(QKeySequence::SelectAll)) {
        m_scene->selectAllNodes();
        e->accept();
        return;
    }
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
        m_scene->deleteSelection();
        e->accept();
        return;
    }
    QGraphicsView::keyPressEvent(e);
}

void FlowCanvas::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasFormat("application/x-autoflow-instr")) {
        QString id = QString::fromUtf8(e->mimeData()->data("application/x-autoflow-instr"));
        m_scene->showGhost(id, mapToScene(e->position().toPoint()));
        e->setDropAction(Qt::CopyAction);
        e->accept();
    } else {
        QGraphicsView::dragEnterEvent(e);
    }
}

void FlowCanvas::dragMoveEvent(QDragMoveEvent* e) {
    if (e->mimeData()->hasFormat("application/x-autoflow-instr")) {
        m_scene->moveGhost(mapToScene(e->position().toPoint()));
        e->accept();
    } else QGraphicsView::dragMoveEvent(e);
}

void FlowCanvas::dragLeaveEvent(QDragLeaveEvent* e) {
    m_scene->hideGhost();
    QGraphicsView::dragLeaveEvent(e);
}

void FlowCanvas::dropEvent(QDropEvent* e) {
    if (e->mimeData()->hasFormat("application/x-autoflow-instr")) {
        QString id = QString::fromUtf8(e->mimeData()->data("application/x-autoflow-instr"));
        m_scene->hideGhost();
        QPointF pos = mapToScene(e->position().toPoint());
        m_scene->addNode(id.toStdString(), pos);
        e->setDropAction(Qt::CopyAction);
        e->accept();
    } else {
        QGraphicsView::dropEvent(e);
    }
}

void FlowCanvas::zoomIn() { scale(1.2, 1.2); emit zoomChanged(zoomLevel()); }
void FlowCanvas::zoomOut() { scale(1.0 / 1.2, 1.0 / 1.2); emit zoomChanged(zoomLevel()); }
void FlowCanvas::zoomFit() {
    if (!m_scene || m_scene->items().isEmpty()) return;
    fitInView(m_scene->itemsBoundingRect().adjusted(-60, -60, 60, 60), Qt::KeepAspectRatio);
    emit zoomChanged(zoomLevel());
}

} // namespace autoflow
