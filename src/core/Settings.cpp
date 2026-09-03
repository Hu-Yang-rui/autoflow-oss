#include "Settings.h"

namespace autoflow {

Settings& Settings::instance() {
    static Settings s;
    return s;
}

Settings::Settings()
    : m_settings(QStringLiteral("AutoFlow"), QStringLiteral("AutoFlow")) {
}

QVariant Settings::read(const QString& key, const QVariant& defaultValue) const {
    auto it = m_cache.constFind(key);
    if (it != m_cache.constEnd()) return *it;
    QVariant v = m_settings.value(key, defaultValue);
    m_cache.insert(key, v);
    return v;
}

void Settings::write(const QString& key, const QVariant& value) {
    m_cache.insert(key, value);
    m_settings.setValue(key, value);
}

// ---- General ----
QString Settings::language() const { return read(QStringLiteral("general/language"), QStringLiteral("zh")).toString(); }
void Settings::setLanguage(const QString& v) { write(QStringLiteral("general/language"), v); }
int Settings::fontScale() const { return read(QStringLiteral("general/fontScale"), 100).toInt(); }
void Settings::setFontScale(int v) { write(QStringLiteral("general/fontScale"), v); }
bool Settings::followSystemTheme() const { return read(QStringLiteral("general/followSystemTheme"), false).toBool(); }
void Settings::setFollowSystemTheme(bool v) { write(QStringLiteral("general/followSystemTheme"), v); }
bool Settings::devMode() const { return read(QStringLiteral("general/devMode"), false).toBool(); }
void Settings::setDevMode(bool v) { write(QStringLiteral("general/devMode"), v); }

// ---- AI ----
QString Settings::aiProvider() const { return read(QStringLiteral("ai/provider"), QString()).toString(); }
void Settings::setAiProvider(const QString& v) { write(QStringLiteral("ai/provider"), v); }
QString Settings::aiBaseUrl() const { return read(QStringLiteral("ai/baseUrl"), QString()).toString(); }
void Settings::setAiBaseUrl(const QString& v) { write(QStringLiteral("ai/baseUrl"), v); }
QString Settings::aiApiKey() const { return read(QStringLiteral("ai/apiKey"), QString()).toString(); }
void Settings::setAiApiKey(const QString& v) { write(QStringLiteral("ai/apiKey"), v); }
QString Settings::aiModel() const { return read(QStringLiteral("ai/model"), QString()).toString(); }
void Settings::setAiModel(const QString& v) { write(QStringLiteral("ai/model"), v); }

// ---- AI 备选方案 ----
bool Settings::aiBackupEnabled() const { return read(QStringLiteral("ai/backupEnabled"), false).toBool(); }
void Settings::setAiBackupEnabled(bool v) { write(QStringLiteral("ai/backupEnabled"), v); }
QString Settings::aiBackupProvider() const { return read(QStringLiteral("ai/backupProvider"), QStringLiteral("custom")).toString(); }
void Settings::setAiBackupProvider(const QString& v) { write(QStringLiteral("ai/backupProvider"), v); }
QString Settings::aiBackupBaseUrl() const { return read(QStringLiteral("ai/backupBaseUrl"), QString()).toString(); }
void Settings::setAiBackupBaseUrl(const QString& v) { write(QStringLiteral("ai/backupBaseUrl"), v); }
QString Settings::aiBackupApiKey() const { return read(QStringLiteral("ai/backupApiKey"), QString()).toString(); }
void Settings::setAiBackupApiKey(const QString& v) { write(QStringLiteral("ai/backupApiKey"), v); }
QString Settings::aiBackupModel() const { return read(QStringLiteral("ai/backupModel"), QString()).toString(); }
void Settings::setAiBackupModel(const QString& v) { write(QStringLiteral("ai/backupModel"), v); }

// ---- Hotkey ----
QString Settings::hotkeyRun() const { return read(QStringLiteral("hotkey/run"), QStringLiteral("F10")).toString(); }
void Settings::setHotkeyRun(const QString& v) { write(QStringLiteral("hotkey/run"), v); }
QString Settings::hotkeyStop() const { return read(QStringLiteral("hotkey/stop"), QStringLiteral("F9")).toString(); }
void Settings::setHotkeyStop(const QString& v) { write(QStringLiteral("hotkey/stop"), v); }

// ---- Execution ----
int Settings::execMaxSteps() const { return read(QStringLiteral("exec/maxSteps"), 0).toInt(); }
void Settings::setExecMaxSteps(int v) { write(QStringLiteral("exec/maxSteps"), v); }
int Settings::execDefaultRetry() const { return read(QStringLiteral("exec/defaultRetry"), 0).toInt(); }
void Settings::setExecDefaultRetry(int v) { write(QStringLiteral("exec/defaultRetry"), v); }
int Settings::inputClickHoldMs() const { return read(QStringLiteral("exec/inputClickHoldMs"), 20).toInt(); }
void Settings::setInputClickHoldMs(int v) { write(QStringLiteral("exec/inputClickHoldMs"), v); }
int Settings::inputTypeIntervalMs() const { return read(QStringLiteral("exec/inputTypeIntervalMs"), 20).toInt(); }
void Settings::setInputTypeIntervalMs(int v) { write(QStringLiteral("exec/inputTypeIntervalMs"), v); }
bool Settings::notifyOnFinish() const { return read(QStringLiteral("exec/notifyOnFinish"), false).toBool(); }
void Settings::setNotifyOnFinish(bool v) { write(QStringLiteral("exec/notifyOnFinish"), v); }
bool Settings::showRunBorder() const { return read(QStringLiteral("exec/showRunBorder"), true).toBool(); }
void Settings::setShowRunBorder(bool v) { write(QStringLiteral("exec/showRunBorder"), v); }

// ---- Network ----
int Settings::httpTimeoutMs() const { return read(QStringLiteral("net/httpTimeoutMs"), 10000).toInt(); }
void Settings::setHttpTimeoutMs(int v) { write(QStringLiteral("net/httpTimeoutMs"), v); }
int Settings::httpRetry() const { return read(QStringLiteral("net/httpRetry"), 0).toInt(); }
void Settings::setHttpRetry(int v) { write(QStringLiteral("net/httpRetry"), v); }

// ---- OCR ----
QString Settings::ocrLanguage() const { return read(QStringLiteral("ocr/language"), QStringLiteral("chi_sim+eng")).toString(); }
void Settings::setOcrLanguage(const QString& v) { write(QStringLiteral("ocr/language"), v); }

// ---- Screenshot ----
QString Settings::shotDir() const { return read(QStringLiteral("shot/dir"), QString()).toString(); }
void Settings::setShotDir(const QString& v) { write(QStringLiteral("shot/dir"), v); }
bool Settings::shotAutoClean() const { return read(QStringLiteral("shot/autoClean"), false).toBool(); }
void Settings::setShotAutoClean(bool v) { write(QStringLiteral("shot/autoClean"), v); }

// ---- Files ----
QString Settings::filesDefaultDir() const { return read(QStringLiteral("files/defaultDir"), QString()).toString(); }
void Settings::setFilesDefaultDir(const QString& v) { write(QStringLiteral("files/defaultDir"), v); }
int Settings::recentMax() const { return read(QStringLiteral("files/recentMax"), 8).toInt(); }
void Settings::setRecentMax(int v) { write(QStringLiteral("files/recentMax"), v); }
QStringList Settings::recentFiles() const { return read(QStringLiteral("files/recent"), QStringList()).toStringList(); }
void Settings::setRecentFiles(const QStringList& v) { write(QStringLiteral("files/recent"), v); }
QString Settings::lastDir() const { return read(QStringLiteral("files/lastDir"), QString()).toString(); }
void Settings::setLastDir(const QString& v) { write(QStringLiteral("files/lastDir"), v); }

// ---- Log ----
int Settings::logMaxLines() const { return read(QStringLiteral("log/maxLines"), 0).toInt(); }
void Settings::setLogMaxLines(int v) { write(QStringLiteral("log/maxLines"), v); }
bool Settings::logToFile() const { return read(QStringLiteral("log/toFile"), false).toBool(); }
void Settings::setLogToFile(bool v) { write(QStringLiteral("log/toFile"), v); }

// ---- Autostart ----
bool Settings::autostart() const { return read(QStringLiteral("system/autostart"), false).toBool(); }
void Settings::setAutostart(bool v) { write(QStringLiteral("system/autostart"), v); }

} // namespace autoflow
