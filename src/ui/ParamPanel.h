#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QHash>
#include "../core/FlowModel.h"
#include "../instructions/IInstruction.h"

class QVBoxLayout;
class QLabel;
class QComboBox;
class QCheckBox;
class QLineEdit;

namespace autoflow {

class Stepper;

// 右侧参数面板：展示选中步骤的配置项（带中文说明与示例值）
class ParamPanel : public QWidget {
    Q_OBJECT
public:
    explicit ParamPanel(FlowModel* model, QWidget* parent = nullptr);

    void setNode(const QString& nodeId);
    QString currentNodeId() const { return m_nodeId; }
    void clearNode();

    // 当前节点参数是否全部合法（必填非空、数值未越界）；供运行前汇总校验
    bool validateCurrent() const;

signals:
    void paramEdited(const QString& nodeId);

private:
    void rebuild();
    void addParamRow(const IInstruction::Param& p, QVBoxLayout* lay);
    json readParam(const QString& key) const;
    void setParamWidget(QWidget* w, const std::string& type, const json& value, const std::string& def);
    void writeBack(const QString& key);
    // 就地校验：更新错误红框 + 行内提示（必填为空 / 数值越界）
    void validateParam(const QString& key);
    bool paramIsEmpty(const QString& key) const;
    void setParamError(const QString& key, bool invalid, const QString& msg);
    QString labelText(const QString& base, bool required) const;
    // 截图裁剪选取模板图片：隐藏主窗口 → 截全屏 → 框选 → 裁剪保存 → 回填路径
    void pickTemplateFromScreen(QLineEdit* le, const QString& key);

    FlowModel* m_model = nullptr;
    QString m_nodeId;
    const IInstruction* m_instr = nullptr;
    bool m_updating = false;

    QScrollArea* m_scroll = nullptr;
    QWidget* m_content = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_desc = nullptr;
    QHash<QString, QWidget*> m_widgets;
    QHash<QString, QLabel*> m_labels;   // 参数标签（用于 if 预设联动时动态改文案）
    QHash<QString, QLabel*> m_errors;   // 行内错误提示标签
    QHash<QString, IInstruction::Param> m_paramMeta;   // 参数元数据（required/type/hint 等，供校验）
    QComboBox* m_onError = nullptr;
    Stepper* m_retry = nullptr;
    QLineEdit* m_comment = nullptr;
};

} // namespace autoflow
