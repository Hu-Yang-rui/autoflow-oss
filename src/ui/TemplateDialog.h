#pragma once
#include <QDialog>
#include <QVector>
#include <QString>

namespace autoflow {

// 模板条目：名称 + 描述 + 节点链（如 "开始 → 找图 → 结束"）
struct TemplateItem {
    QString name;
    QString desc;
    QString chain;
};

// 模板选择对话框：每个模板一个可点击按钮，点击后 accept() 并记录索引
class TemplateDialog : public QDialog {
    Q_OBJECT
public:
    explicit TemplateDialog(const QVector<TemplateItem>& items, QWidget* parent = nullptr);

    int selectedIndex() const { return m_selected; }   // 未选中为 -1

signals:
    void newTemplateRequested();
    void openTemplateRequested();
    void saveTemplateRequested();

private:
    int m_selected = -1;
};

} // namespace autoflow
