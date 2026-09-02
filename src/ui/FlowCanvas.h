#pragma once
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHash>
#include <QSet>
#include <QVector>
#include "../core/FlowModel.h"
#include "../instructions/InstructionRegistry.h"
#include "NodeItem.h"
#include "EdgeItem.h"

class QTimer;

namespace autoflow {

// 流程画布场景：管理节点/连线、拖放、连接、框选删除
class FlowCanvasScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit FlowCanvasScene(FlowModel* model, QObject* parent = nullptr);

    void setDark(bool dark) { m_dark = dark; update(); }
    bool isDark() const { return m_dark; }

    void loadFromModel();
    void addNode(const std::string& instrId, const QPointF& pos);
    void deleteSelection();
    void deleteNode(const QString& id);              // 删除单个节点（右键菜单）
    void duplicateNode(const QString& id);           // 复制节点（右键菜单）
    void setNodeEnabled(const QString& id, bool on); // 启用/禁用节点
    void refreshNodeSummary(const QString& id);
    void highlightNode(const QString& id);   // 运行时高亮当前节点
    void clearHighlight();
    qreal runProgress() const { return m_runProgress; }   // 运行脉冲进度 0..1

    // 运行结果可视化：标记节点运行状态（none/running/ok/error），运行结束不清除
    void markNodeState(const QString& id, const QString& state);
    void clearRunStates();

    // 撤销/重做（模型 JSON 快照命令栈）
    void undo();
    void redo();
    bool canUndo() const { return !m_undoStack.isEmpty(); }
    bool canRedo() const { return !m_redoStack.isEmpty(); }
    json snapshotJson() const { return m_model ? m_model->toJson() : json(); }
    void pushUndoSnapshot(const json& before);          // 移动等：已捕获的“改动前”快照入栈
    void recordUndoPoint();                             // 即将改动前：捕获当前快照入栈

    // 复制 / 粘贴 / 全选（Ctrl+C / Ctrl+V / Ctrl+A）
    void copySelection();
    void pasteClipboard();
    void selectAllNodes();

    // 拖拽指令时的幽灵预览
    void showGhost(const QString& instrId, const QPointF& scenePos);
    void moveGhost(const QPointF& scenePos);
    void hideGhost();

    // 端口连接拖动
    void beginConnection(PortItem* from);
    void updateConnection(const QPointF& scenePos);
    void endConnection(const QPointF& scenePos);
    void onNodeMoved(NodeItem* node);

signals:
    void nodeSelected(const QString& id);
    void modelChanged();
    void requestRunFrom(const QString& id);   // 右键"从此运行"
    void statusMessage(const QString& msg);   // 状态栏一次性提示（拖线落空等）
    void undoAvailable(bool canUndo);
    void redoAvailable(bool canRedo);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

private slots:
    void onSelectionChanged();
    void onPulseTick();

private:
    void connectPorts(PortItem* from, PortItem* to);
    void removeEdgeItem(const QString& from, const QString& label);
    void autoChain(NodeItem* item);
    void rebuildEdges();
    void rebuildFromModel();                 // 从模型重建场景项（不动撤销栈）
    void restoreSnapshot(const json& j);     // 撤销/重做时恢复快照
    void clearConnectHints();                // 清除拖线时的端口高亮
    PortItem* portAt(const QPointF& scenePos);
    void endParamSession();                  // 结束参数编辑合并会话

    FlowModel* m_model = nullptr;
    QHash<QString, NodeItem*> m_items;
    QList<EdgeItem*> m_edges;
    PortItem* m_tempFrom = nullptr;
    QGraphicsPathItem* m_tempEdge = nullptr;
    QGraphicsItem* m_ghost = nullptr;        // 拖拽幽灵预览
    bool m_dark = false;
    QTimer* m_pulseTimer = nullptr;
    qreal m_runProgress = 0;
    QString m_runningId;

    // 撤销/重做快照栈（保存改动前的模型 JSON）
    QVector<json> m_undoStack;
    QVector<json> m_redoStack;

    // 参数写回合并：同一选中节点连续编辑合并为一个撤销点
    QString m_paramEditNode;
    json m_paramEditBefore;
    bool m_paramEditActive = false;
    bool m_paramEditPushed = false;
};

// 画布视图：缩放、框选、拖放指令
class FlowCanvas : public QGraphicsView {
    Q_OBJECT
public:
    explicit FlowCanvas(FlowModel* model, QWidget* parent = nullptr);

    FlowCanvasScene* canvasScene() const { return m_scene; }
    void setDark(bool dark) { m_scene->setDark(dark); }
    void zoomIn();
    void zoomOut();
    void zoomFit();

    // 当前缩放比例（1.0 = 100%），缩放变化时 emit zoomChanged
    qreal zoomLevel() const { return transform().m11(); }

signals:
    void zoomChanged(qreal level);

protected:
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void dragLeaveEvent(QDragLeaveEvent* e) override;

private:
    FlowCanvasScene* m_scene = nullptr;
};

} // namespace autoflow
