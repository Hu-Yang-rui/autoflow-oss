#pragma once
#include <QWidget>
#include <QTreeWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMimeData>

namespace autoflow {

// 可拖拽的指令树（分类 → 指令）
class InstructionTree : public QTreeWidget {
    Q_OBJECT
public:
    explicit InstructionTree(QWidget* parent = nullptr);

protected:
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QList<QTreeWidgetItem*>& items) const override;
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool viewportEvent(QEvent* event) override;

private:
    QPersistentModelIndex m_hoverIndex;   // 跟踪当前 hover 的 item（HoverLeave 时清空，drawRow 用它判断 MouseOver）
};

// 左侧指令面板
class InstructionPanel : public QWidget {
    Q_OBJECT
public:
    explicit InstructionPanel(QWidget* parent = nullptr);

private:
    InstructionTree* m_tree = nullptr;
};

} // namespace autoflow
