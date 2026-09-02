#pragma once
#include <QGraphicsItem>
#include <QVector>
#include <QColor>
#include "../core/FlowModel.h"
#include "../instructions/IInstruction.h"

namespace autoflow {

class FlowCanvasScene;
class PortItem;

// 画布节点卡片
class NodeItem : public QGraphicsItem {
public:
    enum { Type = UserType + 1 };

    NodeItem(FlowModel* model, const IInstruction* instr, const QString& nodeId,
             QGraphicsItem* parent = nullptr);

    int type() const override { return Type; }
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

    QString nodeId() const { return m_nodeId; }
    const IInstruction* instr() const { return m_instr; }
    FlowNode* flowNode() const;
    bool nodeEnabled() const;                    // 读 FlowNode.enabled
    void setRunning(bool on);
    bool isRunning() const { return m_running; }
    void refreshSummary();
    void setRunState(const QString&) { update(); }   // 运行时状态变化时重绘

    PortItem* inputPort() const { return m_input; }
    const QVector<PortItem*>& outputPorts() const { return m_outputs; }
    QPointF inputScenePos() const;
    QPointF outputScenePos(const QString& label) const;
    QPointF outputScenePos(int index) const;

    static QColor categoryColor(const std::string& category);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    void layoutPorts();
    QString summaryText() const;
    qreal nodeWidth() const { return 200; }
    qreal nodeHeight() const;

    FlowModel* m_model = nullptr;
    QString m_nodeId;
    const IInstruction* m_instr = nullptr;
    PortItem* m_input = nullptr;
    QVector<PortItem*> m_outputs;
    bool m_running = false;

    // 移动撤销：按下时捕获“移动前”快照，松手且位置变化后入撤销栈
    QPointF m_moveBeforePos;
    json m_moveBeforeSnapshot;
    bool m_moveCaptured = false;
};

// 端口（输入/输出）小圆点
class PortItem : public QGraphicsItem {
public:
    enum { Type = UserType + 2 };

    // 拖线期间的端口提示态
    enum ConnectHint { HintNone = 0, HintValid = 1, HintInvalid = 2 };

    PortItem(NodeItem* owner, bool isInput, const QString& label,
             QGraphicsItem* parent = nullptr);

    int type() const override { return Type; }
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

    bool isInput() const { return m_input; }
    QString label() const { return m_label; }
    NodeItem* owner() const { return m_owner; }
    int connectHint() const { return m_connectHint; }
    void setConnectHint(int h) { m_connectHint = h; update(); }
    void setDragActive(bool on) { m_dragActive = on; update(); }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e) override;

private:
    NodeItem* m_owner = nullptr;
    bool m_input = false;
    QString m_label;
    bool m_dragging = false;
    bool m_hover = false;
    int m_connectHint = HintNone;
    bool m_dragActive = false;
};

} // namespace autoflow
