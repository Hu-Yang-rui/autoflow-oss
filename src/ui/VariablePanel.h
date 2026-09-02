#pragma once
#include <QWidget>
#include <QTreeWidget>

namespace autoflow {

// 变量运行面板（实时查看全局变量与步骤返回值）
class VariablePanel : public QWidget {
    Q_OBJECT
public:
    explicit VariablePanel(QWidget* parent = nullptr);

public slots:
    void setSnapshot(const QString& json);
    void clearVars();

private:
    QTreeWidget* m_tree = nullptr;
};

} // namespace autoflow
