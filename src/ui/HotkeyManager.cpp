#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "HotkeyManager.h"
#include "../core/Settings.h"

#include <windows.h>

#include <QAbstractNativeEventFilter>
#include <QCoreApplication>
#include <QString>
#include <QMessageBox>
#include <QDebug>
#include <functional>

namespace autoflow {

namespace {

// 解析热键字符串（如 "F10"、"Ctrl+F1"、"Alt+Shift+A"）为 Win32 vk + fsModifiers。
// 支持：F1..F24、单字母 A..Z、单数字 0..9，前缀 Ctrl+/Alt+/Shift+ 可组合，大小写不敏感。
// 解析失败返回 false（调用方回退默认键）。
bool parseHotkey(const QString& text, UINT& vkOut, UINT& modsOut) {
    QString s = text.trimmed();
    if (s.isEmpty()) return false;

    UINT mods = 0;   // 不用 MOD_NOREPEAT：部分环境下单独使用会导致 RegisterHotKey 失败（如 F12）
    // 逐个剥离修饰键前缀
    bool stripped;
    do {
        stripped = false;
        struct { const char* prefix; UINT mod; } entries[] = {
            {"Ctrl+", MOD_CONTROL}, {"Alt+", MOD_ALT}, {"Shift+", MOD_SHIFT},
        };
        for (const auto& e : entries) {
            const QString p = QString::fromLatin1(e.prefix);
            if (s.startsWith(p, Qt::CaseInsensitive)) {
                mods |= e.mod;
                s = s.mid(p.size()).trimmed();
                stripped = true;
                break;
            }
        }
    } while (stripped);

    if (s.isEmpty()) return false;

    UINT vk = 0;
    // F1..F24
    if (s.size() >= 2 && s.at(0).toUpper() == QLatin1Char('F')) {
        bool ok = false;
        const int n = s.mid(1).toInt(&ok);
        if (!ok || n < 1 || n > 24) return false;
        vk = VK_F1 + (n - 1);
    } else if (s.size() == 1) {
        const QChar c = s.at(0).toUpper();
        if (c >= QLatin1Char('A') && c <= QLatin1Char('Z'))
            vk = c.toLatin1(); // 'A'..'Z' 与 VK 码一致
        else if (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
            vk = c.toLatin1(); // '0'..'9' 与 VK 码一致
        else
            return false;
    } else {
        return false;
    }

    vkOut = vk;
    modsOut = mods;
    return true;
}

} // namespace

class HotkeyNativeFilter : public QAbstractNativeEventFilter {
public:
    std::function<void(int)> onHotkey;
    bool nativeEventFilter(const QByteArray&, void* message, qintptr*) override {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY) {
            if (onHotkey) onHotkey((int)msg->wParam);
            return true;
        }
        return false;
    }
};

HotkeyManager::HotkeyManager(QObject* parent) : QObject(parent) {
    auto* f = new HotkeyNativeFilter();
    f->onHotkey = [this](int id) {
        if (id == 1) emit runRequested();
        else if (id == 2) emit stopRequested();
    };
    m_filter = f;
    QCoreApplication::instance()->installNativeEventFilter(m_filter);
}

HotkeyManager::~HotkeyManager() {
    UnregisterHotKey(nullptr, 1);
    UnregisterHotKey(nullptr, 2);
    if (QCoreApplication::instance())
        QCoreApplication::instance()->removeNativeEventFilter(m_filter);
    delete m_filter;
}

bool HotkeyManager::registerHotkeys() {
    // 先释放旧注册，支持“禁用(清空)=不注册”与重新注册
    UnregisterHotKey(nullptr, 1);
    UnregisterHotKey(nullptr, 2);

    // hwnd=nullptr：与当前线程关联，WM_HOTKEY 进入线程消息队列
    Settings& settings = Settings::instance();

    UINT runVk = 0, runMods = MOD_NOREPEAT;
    UINT stopVk = 0, stopMods = MOD_NOREPEAT;
    const QString runKey = settings.hotkeyRun().trimmed();
    const QString stopKey = settings.hotkeyStop().trimmed();

    // 空 = 禁用该热键（不注册）；解析失败同样视为无效
    bool runParsed = !runKey.isEmpty() && parseHotkey(runKey, runVk, runMods);
    bool stopParsed = !stopKey.isEmpty() && parseHotkey(stopKey, stopVk, stopMods);

    // 冲突校验：运行/停止解析成同一组合键
    if (runParsed && stopParsed && runVk == stopVk && runMods == stopMods) {
        const QString msg = tr("运行热键与停止热键冲突（%1），停止热键将不注册").arg(runKey);
        qWarning().noquote() << msg;
        QMessageBox::warning(nullptr, tr("热键冲突"), msg);
        stopParsed = false;
    }

    bool ok = true;

    if (runParsed) {
        if (!RegisterHotKey(nullptr, 1, runMods, runVk)) {
            ok = false;
            const QString msg = tr("运行热键 %1 注册失败，可能已被其它程序占用。请到 设置→热键与执行 换一个热键。").arg(runKey);
            qWarning().noquote() << msg;
            QMessageBox::warning(nullptr, tr("热键注册失败"), msg);
        }
    } else if (!runKey.isEmpty()) {
        const QString msg = tr("运行热键 %1 无效，已忽略").arg(runKey);
        qWarning().noquote() << msg;
    }

    if (stopParsed) {
        if (!RegisterHotKey(nullptr, 2, stopMods, stopVk)) {
            ok = false;
            const QString msg = tr("停止热键 %1 注册失败，可能已被其它程序占用。请到 设置→热键与执行 换一个热键。").arg(stopKey);
            qWarning().noquote() << msg;
            QMessageBox::warning(nullptr, tr("热键注册失败"), msg);
        }
    } else if (!stopKey.isEmpty()) {
        const QString msg = tr("停止热键 %1 无效，已忽略").arg(stopKey);
        qWarning().noquote() << msg;
    }

    return ok;
}

} // namespace autoflow
