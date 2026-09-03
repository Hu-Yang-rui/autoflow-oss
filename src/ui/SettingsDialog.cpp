#include "SettingsDialog.h"
#include "Stepper.h"
#include "ThemeManager.h"
#include "ThemeToggle.h"
#include "MainWindow.h"             // MainWindow::applyAutostart
#include "../core/Settings.h"
#include "../infra/HttpClient.h"
#include "../common.h"              // nlohmann::json

#include <QTabWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequenceEdit>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QLocale>
#include <algorithm>
#include <vector>
#include <utility>

namespace autoflow {

namespace {
// 智谱预设（与 VisionInstructions.cpp resolveAiEndpoint 保持一致）
constexpr const char* kZhipuBaseUrl = "https://open.bigmodel.cn/api/paas/v4/chat/completions";
constexpr const char* kZhipuModel   = "glm-4-flash";
// Ollama 本地大模型预设（[OI] 兼容端点）
constexpr const char* kOllamaBaseUrl = "http://localhost:11434/v1/chat/completions";
constexpr const char* kOllamaModel   = "llama3.2-vision";
// LM Studio 本地大模型预设（OpenAI 兼容端点）
constexpr const char* kLmStudioBaseUrl = "http://localhost:1234/v1/chat/completions";
constexpr const char* kLmStudioModel   = "local-model";
} // namespace

SettingsDialog::SettingsDialog(ThemeToggle* themeToggle, QWidget* parent)
    : QDialog(parent), m_themeToggle(themeToggle) {
    setWindowTitle(tr("设置"));
    setMinimumWidth(520);

    auto* lay = new QVBoxLayout(this);
    auto* tabs = new QTabWidget(this);
    tabs->addTab(buildGeneralTab(),  tr("通用"));
    tabs->addTab(buildAiTab(),       tr("AI 服务"));
    tabs->addTab(buildHotkeyTab(),   tr("热键与执行"));
    tabs->addTab(buildNetworkTab(),  tr("网络与识别"));
    tabs->addTab(buildFilesTab(),    tr("文件与日志"));
    // 修复：5 个 tab 全部平铺可见，不出现左右滚动箭头（无需点箭头才能看到其它 tab）
    tabs->setUsesScrollButtons(false);
    tabs->tabBar()->setExpanding(true);
    tabs->tabBar()->setElideMode(Qt::ElideNone);
    lay->addWidget(tabs);

    // 重启生效提示（语言 / 热键等带 * 的项）
    auto* restartHint = new QLabel(tr("带 * 的设置在重启后生效"), this);
    lay->addWidget(restartHint);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    lay->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] { accept(); });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// ================================ 通用 ================================
QWidget* SettingsDialog::buildGeneralTab() {
    Settings& s = Settings::instance();
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    // 界面语言（重启后生效）：只列出实际有翻译的语言
    m_langCombo = new QComboBox(page);
    m_langCombo->addItem(QStringLiteral("中文 (zh)"), QStringLiteral("zh"));
    m_langCombo->addItem(QStringLiteral("English (en)"), QStringLiteral("en"));
    const int langIdx = m_langCombo->findData(s.language());
    if (langIdx >= 0) m_langCombo->setCurrentIndex(langIdx);
    auto* langWrap = new QWidget(page);
    auto* langLay = new QHBoxLayout(langWrap);
    langLay->setContentsMargins(0, 0, 0, 0);
    langLay->addWidget(m_langCombo);
    m_langNote = new QLabel(tr("重启后生效"), langWrap);
    m_langNote->setVisible(false);
    langLay->addWidget(m_langNote);
    langLay->addStretch(1);
    form->addRow(tr("界面语言 *"), langWrap);
    connect(m_langCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        // 与当前生效语言不同时提示重启
        m_langNote->setVisible(m_langCombo->currentData().toString() != Settings::instance().language());
    });

    // 字体缩放（百分比；Stepper 无 suffix，单位写在 label 里）
    m_fontScaleSpin = new Stepper(page);
    m_fontScaleSpin->setRange(50, 200);
    m_fontScaleSpin->setDecimals(0);
    m_fontScaleSpin->setSingleStep(10);
    m_fontScaleSpin->setValue((double)s.fontScale());
    form->addRow(tr("字体缩放 (%)"), m_fontScaleSpin);

    // 主题三态：浅色 / 深色 / 跟随系统（即时生效）
    m_themeCombo = new QComboBox(page);
    m_themeCombo->addItem(tr("浅色"));
    m_themeCombo->addItem(tr("深色"));
    m_themeCombo->addItem(tr("跟随系统"));
    if (s.followSystemTheme())
        m_themeCombo->setCurrentIndex(2);
    else
        m_themeCombo->setCurrentIndex(ThemeManager::instance().isDark() ? 1 : 0);
    form->addRow(tr("主题"), m_themeCombo);
    connect(m_themeCombo, &QComboBox::currentIndexChanged, this, &SettingsDialog::applyThemeIndex);

    // 开机自启（即时应用注册表 Run 键）
    m_autostartBox = new QCheckBox(tr("开机自动启动 AutoFlow"), page);
    m_autostartBox->setChecked(s.autostart());
    form->addRow(tr("开机自启"), m_autostartBox);
    connect(m_autostartBox, &QCheckBox::toggled, this, [](bool on) {
        Settings::instance().setAutostart(on);
        MainWindow::applyAutostart(on);
    });

    // 不再显示新手教程（沿用旧对话框的 QSettings 键 tutorial/dontShow）
    m_noTutorialBox = new QCheckBox(tr("不再显示新手教程"), page);
    m_noTutorialBox->setChecked(QSettings("AutoFlow", "AutoFlow")
                                    .value("tutorial/dontShow", false).toBool());
    form->addRow(tr("新手教程"), m_noTutorialBox);

    // 开发者模式：开启后显示开发者功能（ULW）
    m_devModeBox = new QCheckBox(tr("开发者模式"), page);
    m_devModeBox->setChecked(s.devMode());
    m_devModeBox->setToolTip(tr("勾选后显示开发者功能"));
    form->addRow(tr("开发者模式"), m_devModeBox);

    return page;
}

// 主题三态即时应用：与旧 openSettings() 的 darkBox 逻辑一致——
// ThemeManager::setDark 立即生效 + 同步 ThemeToggle + 写 QSettings "theme/dark"。
void SettingsDialog::applyThemeIndex(int idx) {
    Settings& s = Settings::instance();
    ThemeManager& tm = ThemeManager::instance();

    bool dark;
    if (idx == 2) {
        // 跟随系统：先写标记，effectiveDark() 随之变为系统主题；
        // 再 setDark(生效值)——若与当前显式值不同则触发 apply() 立即换肤，
        // 相同则说明当前皮肤已是系统主题，无需重复应用。
        s.setFollowSystemTheme(true);
        dark = tm.effectiveDark();
    } else {
        s.setFollowSystemTheme(false);
        dark = (idx == 1);
    }
    tm.setDark(dark);
    if (m_themeToggle) {
        // 程序化同步开关：抑制 toggled 副作用，避免 MainWindow 的 toggled 槽
        // 把 followSystemTheme 误清为 false（覆盖这里刚写入的"跟随系统"状态）
        const bool wasBlocked = m_themeToggle->blockSignals(true);
        m_themeToggle->setDark(dark);
        m_themeToggle->blockSignals(wasBlocked);
        // blockSignals 期间 toggled→animateProgress 不触发，这里直接同步视觉进度，
        // 确保无论信号是否被屏蔽，滑块/图标都反映实际主题
        m_themeToggle->setProgress(dark ? 1.0 : 0.0);
    }
    QSettings("AutoFlow", "AutoFlow").setValue("theme/dark", dark);
}

// ================================ AI 服务 ================================
QWidget* SettingsDialog::buildAiTab() {
    Settings& s = Settings::instance();
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    // 服务商
    m_providerCombo = new QComboBox(page);
    m_providerCombo->addItem(tr("Ollama（本地）"), QStringLiteral("ollama"));
    m_providerCombo->addItem(tr("LM Studio（本地）"), QStringLiteral("lmstudio"));
    m_providerCombo->addItem(tr("自定义"), QStringLiteral("custom"));
    int provIdx = m_providerCombo->findData(s.aiProvider());
    if (provIdx < 0) provIdx = m_providerCombo->findData("custom");   // 找不到（如旧的 zhipu）→ 默认自定义
    m_providerCombo->setCurrentIndex(provIdx);
    form->addRow(tr("服务商"), m_providerCombo);
    connect(m_providerCombo, &QComboBox::currentIndexChanged, this,
            [this](int) { updateAiPlaceholders(); });

    // Base URL（留空 = 用服务商预设）
    m_baseUrlEdit = new QLineEdit(s.aiBaseUrl(), page);
    m_baseUrlLabel = new QLabel(tr("Base URL"), page);
    form->addRow(m_baseUrlLabel, m_baseUrlEdit);

    // API Key（密码回显）
    m_apiKeyEdit = new QLineEdit(s.aiApiKey(), page);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyLabel = new QLabel(tr("API Key"), page);
    form->addRow(m_apiKeyLabel, m_apiKeyEdit);

    // 模型（留空 = 用服务商预设）
    m_modelEdit = new QLineEdit(s.aiModel(), page);
    m_modelLabel = new QLabel(tr("模型"), page);
    form->addRow(m_modelLabel, m_modelEdit);

    updateAiPlaceholders();

    // 测试连接
    auto* testBtn = new QPushButton(tr("测试连接"), page);
    form->addRow(QString(), testBtn);
    connect(testBtn, &QPushButton::clicked, this, &SettingsDialog::testAiConnection);

    // ---- 备选方案：主服务报错时自动切换 ----
    m_backupEnabledBox = new QCheckBox(tr("启用备选方案（主服务报错时自动切换）"), page);
    form->addRow(tr("备选方案"), m_backupEnabledBox);

    m_backupProviderCombo = new QComboBox(page);
    m_backupProviderCombo->addItem(tr("Ollama（本地）"), QStringLiteral("ollama"));
    m_backupProviderCombo->addItem(tr("LM Studio（本地）"), QStringLiteral("lmstudio"));
    m_backupProviderCombo->addItem(tr("自定义"), QStringLiteral("custom"));
    int bProvIdx = m_backupProviderCombo->findData(s.aiBackupProvider());
    if (bProvIdx < 0) bProvIdx = m_backupProviderCombo->findData("custom");
    m_backupProviderCombo->setCurrentIndex(bProvIdx);
    form->addRow(tr("备选服务商"), m_backupProviderCombo);

    m_backupBaseUrlEdit = new QLineEdit(s.aiBackupBaseUrl(), page);
    m_backupBaseUrlLabel = new QLabel(tr("备选 Base URL"), page);
    form->addRow(m_backupBaseUrlLabel, m_backupBaseUrlEdit);

    m_backupApiKeyEdit = new QLineEdit(s.aiBackupApiKey(), page);
    m_backupApiKeyEdit->setEchoMode(QLineEdit::Password);
    m_backupApiKeyLabel = new QLabel(tr("备选 API Key"), page);
    form->addRow(m_backupApiKeyLabel, m_backupApiKeyEdit);

    m_backupModelEdit = new QLineEdit(s.aiBackupModel(), page);
    m_backupModelLabel = new QLabel(tr("备选模型"), page);
    form->addRow(m_backupModelLabel, m_backupModelEdit);

    m_backupEnabledBox->setChecked(s.aiBackupEnabled());
    connect(m_backupEnabledBox, &QCheckBox::toggled, this,
            [this](bool on) { setBackupFieldsEnabled(on); });
    connect(m_backupProviderCombo, &QComboBox::currentIndexChanged, this,
            [this](int) { updateBackupAiPlaceholders(); });
    updateBackupAiPlaceholders();

    return page;
}

// 按服务商用预设做占位符（仅当输入框为空时显示，不覆盖已填内容；custom 无占位符）
void SettingsDialog::updateAiPlaceholders() {
    const QString provider = m_providerCombo->currentData().toString();
    if (provider.isEmpty() || provider == QLatin1String("zhipu")) {
        m_baseUrlEdit->setPlaceholderText(QString::fromLatin1(kZhipuBaseUrl));
        m_modelEdit->setPlaceholderText(QString::fromLatin1(kZhipuModel));
    } else if (provider == QLatin1String("ollama")) {
        m_baseUrlEdit->setPlaceholderText(QString::fromLatin1(kOllamaBaseUrl));
        m_modelEdit->setPlaceholderText(QString::fromLatin1(kOllamaModel));
    } else if (provider == QLatin1String("lmstudio")) {
        m_baseUrlEdit->setPlaceholderText(QString::fromLatin1(kLmStudioBaseUrl));
        m_modelEdit->setPlaceholderText(QString::fromLatin1(kLmStudioModel));
    } else {
        m_baseUrlEdit->setPlaceholderText(QString());
        m_modelEdit->setPlaceholderText(QString());
    }

    // 本地模型（Ollama / LM Studio）无需 API Key：禁用输入框并提示
    const bool local = (provider == QLatin1String("ollama") || provider == QLatin1String("lmstudio"));
    m_apiKeyEdit->setEnabled(!local);
    m_apiKeyEdit->setPlaceholderText(local ? tr("本地模型无需 API Key") : QString());

    // 智谱（预设服务）：仅 Base URL 隐藏（内置默认），API Key 和模型仍需用户填写
    const bool preset = (provider.isEmpty() || provider == QLatin1String("zhipu"));
    m_baseUrlEdit->setVisible(!preset);
    m_baseUrlLabel->setVisible(!preset);
}

// 备选方案占位符：与主服务商一致（ollama / lmstudio 有预设，custom 无占位符）
void SettingsDialog::updateBackupAiPlaceholders() {
    const QString provider = m_backupProviderCombo->currentData().toString();
    if (provider == QLatin1String("ollama")) {
        m_backupBaseUrlEdit->setPlaceholderText(QString::fromLatin1(kOllamaBaseUrl));
        m_backupModelEdit->setPlaceholderText(QString::fromLatin1(kOllamaModel));
    } else if (provider == QLatin1String("lmstudio")) {
        m_backupBaseUrlEdit->setPlaceholderText(QString::fromLatin1(kLmStudioBaseUrl));
        m_backupModelEdit->setPlaceholderText(QString::fromLatin1(kLmStudioModel));
    } else {
        m_backupBaseUrlEdit->setPlaceholderText(QString());
        m_backupModelEdit->setPlaceholderText(QString());
    }
    const bool local = (provider == QLatin1String("ollama") || provider == QLatin1String("lmstudio"));
    m_backupApiKeyEdit->setPlaceholderText(local ? tr("本地模型无需 API Key") : QString());
    // 统一刷新整组启停（受「启用备选」开关与是否本地共同影响）
    setBackupFieldsEnabled(m_backupEnabledBox->isChecked());
}

// 备选方案整组启停：未启用时全部禁用；启用时仅「本地模型的 API Key」保持禁用
void SettingsDialog::setBackupFieldsEnabled(bool on) {
    const QString provider = m_backupProviderCombo->currentData().toString();
    const bool local = (provider == QLatin1String("ollama") || provider == QLatin1String("lmstudio"));
    m_backupProviderCombo->setEnabled(on);
    m_backupBaseUrlEdit->setEnabled(on);
    m_backupBaseUrlLabel->setEnabled(on);
    m_backupApiKeyEdit->setEnabled(on && !local);
    m_backupApiKeyLabel->setEnabled(on);
    m_backupModelEdit->setEnabled(on);
    m_backupModelLabel->setEnabled(on);
}

// 端点解析与 VisionInstructions.cpp resolveAiEndpoint 一致：
// 空/zhipu 回填智谱预设、ollama 回填本地预设，然后校验 Base URL / 模型非空。
void SettingsDialog::testAiConnection() {
    const QString provider = m_providerCombo->currentData().toString();
    QString baseUrl = m_baseUrlEdit->text().trimmed();
    QString model = m_modelEdit->text().trimmed();
    if (provider.isEmpty() || provider == QLatin1String("zhipu")) {
        if (baseUrl.isEmpty()) baseUrl = QString::fromLatin1(kZhipuBaseUrl);
        if (model.isEmpty()) model = QString::fromLatin1(kZhipuModel);
    } else if (provider == QLatin1String("ollama")) {
        if (baseUrl.isEmpty()) baseUrl = QString::fromLatin1(kOllamaBaseUrl);
        if (model.isEmpty()) model = QString::fromLatin1(kOllamaModel);
        // 用户可能只填服务器根地址，自动补全 OpenAI 兼容端点
        if (!baseUrl.contains("chat/completions")) {
            if (!baseUrl.endsWith('/')) baseUrl += '/';
            baseUrl += "v1/chat/completions";
        }
    } else if (provider == QLatin1String("lmstudio")) {
        if (baseUrl.isEmpty()) baseUrl = QString::fromLatin1(kLmStudioBaseUrl);
        if (model.isEmpty()) model = QString::fromLatin1(kLmStudioModel);
        if (!baseUrl.contains("chat/completions")) {
            if (!baseUrl.endsWith('/')) baseUrl += '/';
            baseUrl += "v1/chat/completions";
        }
    }
    if (baseUrl.isEmpty()) {
        QMessageBox::warning(this, tr("测试连接"), tr("请填写 Base URL"));
        return;
    }
    // 自动补全 /chat/completions：用户可能只填基础端点（如 https://api.a6api.com/v1）
    if (!baseUrl.contains("chat/completions", Qt::CaseInsensitive)) {
        if (!baseUrl.endsWith('/')) baseUrl += '/';
        baseUrl += "chat/completions";
    }
    if (model.isEmpty()) {
        QMessageBox::warning(this, tr("测试连接"), tr("请填写模型名称"));
        return;
    }
    const QString apiKey = m_apiKeyEdit->text().trimmed();
    const bool local = (provider == QLatin1String("ollama") || provider == QLatin1String("lmstudio"));
    if (apiKey.isEmpty() && !local) {
        QMessageBox::warning(this, tr("测试连接"), tr("请填写 API Key"));
        return;
    }

    // 最小 OpenAI 兼容 chat/completions 请求（明确指令 + 限长，避免模型误解"ping"后长文回复超时）
    const json body = {
        { "model", model.toStdString() },
        { "messages", json::array({ { { "role", "user" },
            { "content", tr("请只回复「成功」两个字，不要解释").toStdString() } } }) },
        { "max_tokens", 16 }
    };
    std::map<std::string, std::string> headers;
    if (!apiKey.isEmpty()) headers["Authorization"] = "Bearer " + apiKey.toStdString();
    const HttpResponse res = HttpClient::request("POST", baseUrl.toStdString(), body.dump(),
                                                 "application/json",
                                                 60000, headers);   // 测试用 60 秒超时
    if (res.ok && res.status >= 200 && res.status < 300) {
        QMessageBox::information(this, tr("测试连接"), tr("连接成功"));
    } else {
        // 未启用 OpenSSL 时 https 会失败，如实展示底层错误
        QString detail;
        if (!res.ok) {
            detail = QString::fromStdString(res.error);
        } else {
            detail = tr("HTTP 状态码 %1").arg(res.status);
            if (!res.body.empty())
                detail += QStringLiteral(" — ") +
                          QString::fromStdString(res.body.substr(0, 200));
        }
        QMessageBox::warning(this, tr("测试连接"), tr("连接失败：%1").arg(detail));
    }
}

// ================================ 热键与执行 ================================
QWidget* SettingsDialog::buildHotkeyTab() {
    Settings& s = Settings::instance();
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    // 运行 / 停止热键（保存到 Settings，重启后由 HotkeyManager 注册）
    m_runKeyEdit = new QKeySequenceEdit(QKeySequence(s.hotkeyRun()), page);
    form->addRow(tr("运行热键 *"), m_runKeyEdit);
    m_stopKeyEdit = new QKeySequenceEdit(QKeySequence(s.hotkeyStop()), page);
    form->addRow(tr("停止热键 *"), m_stopKeyEdit);
    auto* hotkeyNote = new QLabel(tr("修改热键将在重启后生效；仅支持 F1..F24、A..Z、0..9，可组合 Ctrl/Alt/Shift"), page);
    hotkeyNote->setWordWrap(true);
    form->addRow(QString(), hotkeyNote);

    // 最大执行步数（0 = 不限；Stepper 无 specialValueText，说明写在 label 里）
    m_maxStepsSpin = new Stepper(page);
    m_maxStepsSpin->setRange(0, 100000);
    m_maxStepsSpin->setDecimals(0);
    m_maxStepsSpin->setSingleStep(1);
    m_maxStepsSpin->setValue((double)s.execMaxSteps());
    form->addRow(tr("最大执行步数 (0=不限)"), m_maxStepsSpin);

    // 默认重试次数
    m_retrySpin = new Stepper(page);
    m_retrySpin->setRange(0, 100);
    m_retrySpin->setDecimals(0);
    m_retrySpin->setSingleStep(1);
    m_retrySpin->setValue((double)s.execDefaultRetry());
    form->addRow(tr("默认重试次数"), m_retrySpin);

    // 点击保持时长 ms
    m_clickHoldSpin = new Stepper(page);
    m_clickHoldSpin->setRange(0, 500);
    m_clickHoldSpin->setDecimals(0);
    m_clickHoldSpin->setSingleStep(1);
    m_clickHoldSpin->setValue((double)s.inputClickHoldMs());
    form->addRow(tr("点击保持时长 (ms)"), m_clickHoldSpin);

    // 字符键入间隔 ms
    m_typeIntervalSpin = new Stepper(page);
    m_typeIntervalSpin->setRange(0, 500);
    m_typeIntervalSpin->setDecimals(0);
    m_typeIntervalSpin->setSingleStep(1);
    m_typeIntervalSpin->setValue((double)s.inputTypeIntervalMs());
    form->addRow(tr("字符键入间隔 (ms)"), m_typeIntervalSpin);

    // 运行完成通知
    m_notifyBox = new QCheckBox(tr("运行完成时提示（提示音 + 状态栏）"), page);
    m_notifyBox->setChecked(s.notifyOnFinish());
    form->addRow(tr("运行完成通知"), m_notifyBox);

    // 执行时屏幕边框
    m_runBorderBox = new QCheckBox(tr("执行时在屏幕四周显示发光边框"), page);
    m_runBorderBox->setChecked(s.showRunBorder());
    form->addRow(tr("执行边框"), m_runBorderBox);

    return page;
}

// QKeySequence(PortableText) -> HotkeyManager::parseHotkey 格式：
// 修饰键规范为 Ctrl+/Alt+/Shift+ 前缀，F 功能键与单字符键大写。
//（parseHotkey 本身大小写不敏感，此处规范化保证写回的字符串稳定一致）
QString SettingsDialog::normalizeHotkey(const QKeySequence& seq) {
    const QString s = seq.toString(QKeySequence::PortableText).trimmed();
    if (s.isEmpty()) return s;

    QStringList out;
    const QStringList parts = s.split(QLatin1Char('+'), Qt::SkipEmptyParts);
    for (QString p : parts) {
        p = p.trimmed();
        const QString low = p.toLower();
        if (low == QLatin1String("ctrl") || low == QLatin1String("control"))
            out << QStringLiteral("Ctrl");
        else if (low == QLatin1String("alt"))
            out << QStringLiteral("Alt");
        else if (low == QLatin1String("shift"))
            out << QStringLiteral("Shift");
        else if (low == QLatin1String("meta"))
            out << QStringLiteral("Meta");
        else if (p.size() >= 2 && (p.at(0) == QLatin1Char('f') || p.at(0) == QLatin1Char('F'))
                 && p.mid(1).toInt() >= 1)
            out << (QStringLiteral("F") + p.mid(1));        // f10 -> F10
        else if (p.size() == 1)
            out << p.toUpper();                              // a -> A
        else
            out << p;                                        // 其余原样（解析失败时 HotkeyManager 回退默认键）
    }
    return out.join(QLatin1Char('+'));
}

// ================================ 网络与识别 ================================
QWidget* SettingsDialog::buildNetworkTab() {
    Settings& s = Settings::instance();
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    // HTTP 超时 ms
    m_httpTimeoutSpin = new Stepper(page);
    m_httpTimeoutSpin->setRange(1000, 120000);
    m_httpTimeoutSpin->setDecimals(0);
    m_httpTimeoutSpin->setSingleStep(1000);
    m_httpTimeoutSpin->setValue((double)s.httpTimeoutMs());
    form->addRow(tr("HTTP 超时 (ms)"), m_httpTimeoutSpin);

    // HTTP 重试次数
    m_httpRetrySpin = new Stepper(page);
    m_httpRetrySpin->setRange(0, 10);
    m_httpRetrySpin->setDecimals(0);
    m_httpRetrySpin->setSingleStep(1);
    m_httpRetrySpin->setValue((double)s.httpRetry());
    form->addRow(tr("HTTP 重试次数"), m_httpRetrySpin);

    // OCR 语言
    m_ocrCombo = new QComboBox(page);
    m_ocrCombo->addItem(tr("中英 (chi_sim+eng)"), QStringLiteral("chi_sim+eng"));
    m_ocrCombo->addItem(tr("中文 (chi_sim)"), QStringLiteral("chi_sim"));
    m_ocrCombo->addItem(tr("英文 (eng)"), QStringLiteral("eng"));
    const int ocrIdx = m_ocrCombo->findData(s.ocrLanguage());
    if (ocrIdx >= 0) m_ocrCombo->setCurrentIndex(ocrIdx);
    form->addRow(tr("OCR 语言"), m_ocrCombo);

    return page;
}

// ================================ 文件与日志 ================================
QWidget* SettingsDialog::buildFilesTab() {
    Settings& s = Settings::instance();
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);

    auto dirRow = [this, page](QLineEdit*& edit, const QString& value, const QString& placeholder) {
        auto* wrap = new QWidget(page);
        auto* h = new QHBoxLayout(wrap);
        h->setContentsMargins(0, 0, 0, 0);
        edit = new QLineEdit(value, wrap);
        edit->setPlaceholderText(placeholder);
        auto* browse = new QPushButton(tr("浏览…"), wrap);
        h->addWidget(edit, 1);
        h->addWidget(browse);
        connect(browse, &QPushButton::clicked, this, [this, edit] {
            const QString dir = QFileDialog::getExistingDirectory(this, tr("选择目录"), edit->text());
            if (!dir.isEmpty()) edit->setText(dir);
        });
        return wrap;
    };

    // 截图目录（空 = 系统临时目录）
    form->addRow(tr("截图目录"), dirRow(m_shotDirEdit, s.shotDir(), tr("留空使用系统临时目录")));

    // 自动清理截图
    m_shotCleanBox = new QCheckBox(tr("退出时自动清理截图"), page);
    m_shotCleanBox->setChecked(s.shotAutoClean());
    form->addRow(tr("自动清理截图"), m_shotCleanBox);

    // 默认流程目录
    form->addRow(tr("默认流程目录"), dirRow(m_filesDirEdit, s.filesDefaultDir(), QString()));

    // 最近文件数量
    m_recentMaxSpin = new Stepper(page);
    m_recentMaxSpin->setRange(0, 20);
    m_recentMaxSpin->setDecimals(0);
    m_recentMaxSpin->setSingleStep(1);
    m_recentMaxSpin->setValue((double)s.recentMax());
    form->addRow(tr("最近文件数量"), m_recentMaxSpin);

    // 日志最大行数（0 = 不限；Stepper 无 specialValueText，说明写在 label 里）
    m_logMaxLinesSpin = new Stepper(page);
    m_logMaxLinesSpin->setRange(0, 100000);
    m_logMaxLinesSpin->setDecimals(0);
    m_logMaxLinesSpin->setSingleStep(1);
    m_logMaxLinesSpin->setValue((double)s.logMaxLines());
    form->addRow(tr("日志最大行数 (0=不限)"), m_logMaxLinesSpin);

    // 日志写入文件
    m_logToFileBox = new QCheckBox(tr("同时将日志写入文件"), page);
    m_logToFileBox->setChecked(s.logToFile());
    form->addRow(tr("日志写入文件"), m_logToFileBox);

    return page;
}

// ================================ 保存 ================================
void SettingsDialog::accept() {
    Settings& s = Settings::instance();

    // 通用（主题 / 开机自启已在变更时即时生效，这里统一写穿保证一致）
    s.setLanguage(m_langCombo->currentData().toString());
    s.setFontScale((int)m_fontScaleSpin->value());
    s.setFollowSystemTheme(m_themeCombo->currentIndex() == 2);
    s.setAutostart(m_autostartBox->isChecked());
    s.setDevMode(m_devModeBox->isChecked());
    QSettings("AutoFlow", "AutoFlow")
        .setValue("tutorial/dontShow", m_noTutorialBox->isChecked());

    // AI 服务
    s.setAiProvider(m_providerCombo->currentData().toString());
    s.setAiBaseUrl(m_baseUrlEdit->text().trimmed());
    s.setAiApiKey(m_apiKeyEdit->text().trimmed());
    s.setAiModel(m_modelEdit->text().trimmed());

    // AI 备选方案
    s.setAiBackupEnabled(m_backupEnabledBox->isChecked());
    s.setAiBackupProvider(m_backupProviderCombo->currentData().toString());
    s.setAiBackupBaseUrl(m_backupBaseUrlEdit->text().trimmed());
    s.setAiBackupApiKey(m_backupApiKeyEdit->text().trimmed());
    s.setAiBackupModel(m_backupModelEdit->text().trimmed());

    // 热键与执行（热键重启后由 HotkeyManager 注册）
    s.setHotkeyRun(normalizeHotkey(m_runKeyEdit->keySequence()));
    s.setHotkeyStop(normalizeHotkey(m_stopKeyEdit->keySequence()));
    s.setExecMaxSteps((int)m_maxStepsSpin->value());
    s.setExecDefaultRetry((int)m_retrySpin->value());
    s.setInputClickHoldMs((int)m_clickHoldSpin->value());
    s.setInputTypeIntervalMs((int)m_typeIntervalSpin->value());
    s.setNotifyOnFinish(m_notifyBox->isChecked());
    s.setShowRunBorder(m_runBorderBox->isChecked());

    // 网络与识别
    s.setHttpTimeoutMs((int)m_httpTimeoutSpin->value());
    s.setHttpRetry((int)m_httpRetrySpin->value());
    s.setOcrLanguage(m_ocrCombo->currentData().toString());

    // 文件与日志
    s.setShotDir(m_shotDirEdit->text().trimmed());
    s.setShotAutoClean(m_shotCleanBox->isChecked());
    s.setFilesDefaultDir(m_filesDirEdit->text().trimmed());
    s.setRecentMax((int)m_recentMaxSpin->value());
    s.setLogMaxLines((int)m_logMaxLinesSpin->value());
    s.setLogToFile(m_logToFileBox->isChecked());

    QDialog::accept();
}

} // namespace autoflow
