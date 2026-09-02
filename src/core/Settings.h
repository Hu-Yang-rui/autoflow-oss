#pragma once
#include <QHash>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace autoflow {

// 全局设置单例：集中读写 QSettings("AutoFlow","AutoFlow")，
// 懒加载 + 内存缓存，setter 立即写穿到 QSettings。
class Settings {
public:
    static Settings& instance();

    // ---- General ----
    QString language() const;                    // "general/language"      默认 "zh"
    void setLanguage(const QString& v);          // 取值 "zh"/"en"
    int fontScale() const;                       // "general/fontScale"     默认 100(百分比)
    void setFontScale(int v);
    bool followSystemTheme() const;              // "general/followSystemTheme" 默认 false
    void setFollowSystemTheme(bool v);
    bool devMode() const;                        // "general/devMode" 默认 false（开发者模式）
    void setDevMode(bool v);

    // ---- AI ----
    QString aiProvider() const;                  // "ai/provider" 默认 ""(空=未配置)
    void setAiProvider(const QString& v);
    QString aiBaseUrl() const;                   // "ai/baseUrl"  默认 ""(空=用 provider 预设)
    void setAiBaseUrl(const QString& v);
    QString aiApiKey() const;                    // "ai/apiKey"   默认 ""
    void setAiApiKey(const QString& v);
    QString aiModel() const;                     // "ai/model"    默认 ""(空=用 provider 预设)
    void setAiModel(const QString& v);

    // ---- AI 备选方案（主服务报错时自动切换）----
    bool aiBackupEnabled() const;                // "ai/backupEnabled"  默认 false
    void setAiBackupEnabled(bool v);
    QString aiBackupProvider() const;            // "ai/backupProvider" 默认 "custom"
    void setAiBackupProvider(const QString& v);
    QString aiBackupBaseUrl() const;             // "ai/backupBaseUrl"  默认 ""
    void setAiBackupBaseUrl(const QString& v);
    QString aiBackupApiKey() const;              // "ai/backupApiKey"   默认 ""
    void setAiBackupApiKey(const QString& v);
    QString aiBackupModel() const;               // "ai/backupModel"    默认 ""
    void setAiBackupModel(const QString& v);

    // ---- Hotkey ----
    QString hotkeyRun() const;                   // "hotkey/run"  默认 "F10"
    void setHotkeyRun(const QString& v);
    QString hotkeyStop() const;                  // "hotkey/stop" 默认 "F12"
    void setHotkeyStop(const QString& v);

    // ---- Execution ----
    int execMaxSteps() const;                    // "exec/maxSteps" 默认 0(0=不限)
    void setExecMaxSteps(int v);
    int execDefaultRetry() const;                // "exec/defaultRetry" 默认 0
    void setExecDefaultRetry(int v);
    int inputClickHoldMs() const;                // "exec/inputClickHoldMs" 默认 20
    void setInputClickHoldMs(int v);
    int inputTypeIntervalMs() const;             // "exec/inputTypeIntervalMs" 默认 20
    void setInputTypeIntervalMs(int v);
    bool notifyOnFinish() const;                 // "exec/notifyOnFinish" 默认 false
    void setNotifyOnFinish(bool v);

    // ---- Network ----
    int httpTimeoutMs() const;                   // "net/httpTimeoutMs" 默认 10000
    void setHttpTimeoutMs(int v);
    int httpRetry() const;                       // "net/httpRetry" 默认 0
    void setHttpRetry(int v);

    // ---- OCR ----
    QString ocrLanguage() const;                 // "ocr/language" 默认 "chi_sim+eng"
    void setOcrLanguage(const QString& v);

    // ---- Screenshot ----
    QString shotDir() const;                     // "shot/dir" 默认 ""(空=系统临时目录+/autoflow)
    void setShotDir(const QString& v);
    bool shotAutoClean() const;                  // "shot/autoClean" 默认 false
    void setShotAutoClean(bool v);

    // ---- Files ----
    QString filesDefaultDir() const;             // "files/defaultDir" 默认 ""
    void setFilesDefaultDir(const QString& v);
    int recentMax() const;                       // "files/recentMax" 默认 8
    void setRecentMax(int v);
    QStringList recentFiles() const;             // "files/recent" 默认 {}（最新在前）
    void setRecentFiles(const QStringList& v);
    QString lastDir() const;                     // "files/lastDir" 默认 ""
    void setLastDir(const QString& v);

    // ---- Log ----
    int logMaxLines() const;                     // "log/maxLines" 默认 0(0=不限)
    void setLogMaxLines(int v);
    bool logToFile() const;                      // "log/toFile" 默认 false
    void setLogToFile(bool v);

    // ---- Autostart ----
    bool autostart() const;                      // "system/autostart" 默认 false
    void setAutostart(bool v);

private:
    Settings();

    // 读：先查内存缓存，未命中则从 QSettings 读(缺省返回 defaultValue)并缓存
    QVariant read(const QString& key, const QVariant& defaultValue) const;
    // 写：更新缓存并立即写穿 QSettings
    void write(const QString& key, const QVariant& value);

    QSettings m_settings;
    mutable QHash<QString, QVariant> m_cache;
};

} // namespace autoflow
