#pragma once
#include <QGraphicsPathItem>
#include <QPainterPath>

namespace autoflow {

class NodeItem;

// 连线（贝塞尔曲线 + 箭头），端点随节点移动
class EdgeItem : public QGraphicsPathItem {
public:
    EdgeItem(NodeItem* from, NodeItem* to, const QString& label, QGraphicsItem* parent = nullptr);

    QString label() const { return m_label; }
    NodeItem* fromNode() const { return m_from; }
    NodeItem* toNode() const { return m_to; }

    void refresh();   // 重新计算路径（节点移动时调用）

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e) override;

private:
    void rebuildPath();
    NodeItem* m_from;
    NodeItem* m_to;
    QString m_label;
    QPainterPath m_path;
    bool m_hover = false;
};

} // namespace autoflow
