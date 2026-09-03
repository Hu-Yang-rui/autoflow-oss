#pragma once
#include <QDialog>
#include "Stepper.h"

class QTabWidget;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QKeySequence;
class QKeySequenceEdit;

namespace autoflow {

class ThemeToggle;

// 设置对话框：5 个分页读写 Settings 单例。
// 主题 / 开机自启 变更即时生效；语言、热键等带 * 的项重启后生效。
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    // themeToggle：主窗口工具栏上的昼夜开关，主题变更时同步其状态（可为空）
    explicit SettingsDialog(ThemeToggle* themeToggle, QWidget* parent = nullptr);

protected:
    void accept() override;   // OK：把所有字段写回 Settings

private:
    QWidget* buildGeneralTab();    // 通用
    QWidget* buildAiTab();         // AI 服务
    QWidget* buildHotkeyTab();     // 热键与执行
    QWidget* buildNetworkTab();    // 网络与识别
    QWidget* buildFilesTab();      // 文件与日志

    void applyThemeIndex(int idx);         // 主题三态即时应用
    void testAiConnection();               // 测试连接（OpenAI 兼容 POST）
    void updateAiPlaceholders();           // 按服务商填充 Base URL / 模型占位符
    void updateBackupAiPlaceholders();     // 备选方案：按服务商填充占位符
    void setBackupFieldsEnabled(bool on);  // 备选方案：启用/禁用整组输入

    // QKeySequence -> HotkeyManager::parseHotkey 接受的格式（"F10"、"Ctrl+F1"…）
    static QString normalizeHotkey(const QKeySequence& seq);

    ThemeToggle* m_themeToggle = nullptr;

    // ---- 通用 ----
    QComboBox* m_langCombo = nullptr;
    QLabel* m_langNote = nullptr;
    Stepper* m_fontScaleSpin = nullptr;
    QComboBox* m_themeCombo = nullptr;
    QCheckBox* m_autostartBox = nullptr;
    QCheckBox* m_noTutorialBox = nullptr;
    QCheckBox* m_devModeBox = nullptr;

    // ---- AI 服务 ----
    QComboBox* m_providerCombo = nullptr;
    QLineEdit* m_baseUrlEdit = nullptr;
    QLabel* m_baseUrlLabel = nullptr;
    QLineEdit* m_apiKeyEdit = nullptr;
    QLabel* m_apiKeyLabel = nullptr;
    QLineEdit* m_modelEdit = nullptr;
    QLabel* m_modelLabel = nullptr;

    // ---- AI 备选方案 ----
    QCheckBox* m_backupEnabledBox = nullptr;
    QComboBox* m_backupProviderCombo = nullptr;
    QLineEdit* m_backupBaseUrlEdit = nullptr;
    QLabel* m_backupBaseUrlLabel = nullptr;
    QLineEdit* m_backupApiKeyEdit = nullptr;
    QLabel* m_backupApiKeyLabel = nullptr;
    QLineEdit* m_backupModelEdit = nullptr;
    QLabel* m_backupModelLabel = nullptr;

    // ---- 热键与执行 ----
    QKeySequenceEdit* m_runKeyEdit = nullptr;
    QKeySequenceEdit* m_stopKeyEdit = nullptr;
    Stepper* m_maxStepsSpin = nullptr;
    Stepper* m_retrySpin = nullptr;
    Stepper* m_clickHoldSpin = nullptr;
    Stepper* m_typeIntervalSpin = nullptr;
    QCheckBox* m_notifyBox = nullptr;
    QCheckBox* m_runBorderBox = nullptr;

    // ---- 网络与识别 ----
    Stepper* m_httpTimeoutSpin = nullptr;
    Stepper* m_httpRetrySpin = nullptr;
    QComboBox* m_ocrCombo = nullptr;

    // ---- 文件与日志 ----
    QLineEdit* m_shotDirEdit = nullptr;
    QCheckBox* m_shotCleanBox = nullptr;
    QLineEdit* m_filesDirEdit = nullptr;
    Stepper* m_recentMaxSpin = nullptr;
    Stepper* m_logMaxLinesSpin = nullptr;
    QCheckBox* m_logToFileBox = nullptr;
};

} // namespace autoflow
