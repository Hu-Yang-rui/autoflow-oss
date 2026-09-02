#pragma once
#include <QObject>

class QAbstractNativeEventFilter;

namespace autoflow {

// 系统级全局热键：运行/停止键由 Settings（hotkey/run、hotkey/stop，默认 F10/F12）配置
// （Win32 RegisterHotKey，即使窗口不在前台也生效）
class HotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit HotkeyManager(QObject* parent = nullptr);
    ~HotkeyManager();

    bool registerHotkeys();

signals:
    void runRequested();
    void stopRequested();

private:
    QAbstractNativeEventFilter* m_filter = nullptr;
};

} // namespace autoflow
