#include "EdgeItem.h"
#include "NodeItem.h"
#include "FlowCanvas.h"
#include "Palette.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QtMath>

namespace autoflow {

EdgeItem::EdgeItem(NodeItem* from, NodeItem* to, const QString& label, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), m_from(from), m_to(to), m_label(label) {
    setZValue(0);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    rebuildPath();
}

void EdgeItem::rebuildPath() {
    QPointF s = m_from->outputScenePos(m_label);
    QPointF e = m_to->inputScenePos();
    if (m_label.isEmpty()) s = m_from->outputScenePos(0);

    qreal dx = qAbs(e.x() - s.x());
    qreal bend = qBound(40.0, dx * 0.5, 180.0);
    QPointF c1(s.x() + bend, s.y());
    QPointF c2(e.x() - bend, e.y());

    QPainterPath path(s);
    path.cubicTo(c1, c2, e);
    m_path = path;
    setPath(path);
}

void EdgeItem::refresh() {
    prepareGeometryChange();
    rebuildPath();
    update();
}

QRectF EdgeItem::boundingRect() const {
    return m_path.boundingRect().adjusted(-8, -8, 8, 8);
}

QPainterPath EdgeItem::shape() const {
    QPainterPathStroker ps;
    ps.setWidth(10);
    return ps.createStroke(m_path);
}

void EdgeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (!m_from || !m_to) return;
    painter->setRenderHint(QPainter::Antialiasing, true);

    bool dark = false;
    FlowCanvasScene* sc = dynamic_cast<FlowCanvasScene*>(scene());
    if (sc) dark = sc->isDark();

    bool live = m_from->isRunning() || m_to->isRunning();

    QColor col = Palette::edge(dark);
    if (m_label == "真") col = Palette::run(dark);
    else if (m_label == "假") col = Palette::stop(dark);
    if (isSelected()) col = Palette::accent(dark);
    if (live) col = Palette::accent(dark);
    if (m_hover && !live && !isSelected()) col = col.lighter(115);

    painter->setPen(QPen(col, isSelected() ? 2.5 : (m_hover ? 2.3 : 1.8)));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_path);

    // 分支标签（真/假/自定义）沿连线中点显示
    if (!m_label.isEmpty() && m_label != "next") {
        QPointF mid = m_path.pointAtPercent(0.5);
        QFont lf = ThemeManager::smileySansFont(); lf.setPixelSize(10);
        QFontMetrics lfm(lf);
        int tw = lfm.horizontalAdvance(m_label) + 12;
        QRectF lr(mid.x() - tw / 2.0, mid.y() - 10, tw, 20);
        painter->setPen(Qt::NoPen);
        painter->setBrush(dark ? Palette::surface2(dark) : Palette::bg(dark));
        painter->drawRoundedRect(lr, 5, 5);
        painter->setPen(col);
        painter->setFont(lf);
        painter->drawText(lr, Qt::AlignCenter, m_label);
    }

    // 箭头
    QPointF e = m_path.pointAtPercent(1.0);
    QPointF ePrev = m_path.pointAtPercent(0.96);
    qreal angle = qAtan2(e.y() - ePrev.y(), e.x() - ePrev.x());
    qreal arrowSize = 8;
    const qreal kPi = 3.14159265358979323846;
    QPointF p1 = e - QPointF(qCos(angle - kPi / 6) * arrowSize, qSin(angle - kPi / 6) * arrowSize);
    QPointF p2 = e - QPointF(qCos(angle + kPi / 6) * arrowSize, qSin(angle + kPi / 6) * arrowSize);
    QPolygonF arrow;
    arrow << e << p1 << p2;
    painter->setPen(Qt::NoPen);
    painter->setBrush(col);
    painter->drawPolygon(arrow);

    // 运行中的信号：沿导线移动的亮点（通电感）
    if (live && sc) {
        qreal t = sc->runProgress();
        QPointF pt = m_path.pointAtPercent(t);
        painter->setPen(Qt::NoPen);
        QColor sig = Palette::accent(dark);
        sig.setAlphaF(0.35f);
        painter->setBrush(sig);
        painter->drawEllipse(pt, 5, 5);
        painter->setBrush(Palette::accent(dark));
        painter->drawEllipse(pt, 2.8, 2.8);
    }
}

void EdgeItem::hoverEnterEvent(QGraphicsSceneHoverEvent*) { m_hover = true; update(); }
void EdgeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*) { m_hover = false; update(); }

} // namespace autoflow
