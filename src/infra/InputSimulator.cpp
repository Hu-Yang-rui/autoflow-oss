#include "InputSimulator.h"
#include "../core/Settings.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <chrono>
#include <thread>
#include <vector>

namespace autoflow {
namespace InputSimulator {

static void sendMouse(int x, int y, DWORD flags, DWORD extraData = 0) {
    SetCursorPos(x, y);
    INPUT in = {};
    in.type = INPUT_MOUSE;
    in.mi.dx = x;
    in.mi.dy = y;
    in.mi.mouseData = extraData;
    in.mi.dwFlags = flags;
    SendInput(1, &in, sizeof(INPUT));
}

bool mouseMove(int x, int y) { return SetCursorPos(x, y) != 0; }

bool mouseMoveSmooth(int x, int y, int durationMs) {
    if (durationMs <= 0) return mouseMove(x, y);
    POINT cur;
    if (!GetCursorPos(&cur)) return mouseMove(x, y);
    int startX = cur.x, startY = cur.y;
    int dx = x - startX, dy = y - startY;
    int steps = durationMs / 10;
    if (steps < 1) steps = 1;
    int stepMs = durationMs / steps;
    for (int i = 1; i <= steps; ++i) {
        int cx = startX + (int)((double)dx * i / steps);
        int cy = startY + (int)((double)dy * i / steps);
        SetCursorPos(cx, cy);
        std::this_thread::sleep_for(std::chrono::milliseconds(stepMs));
    }
    SetCursorPos(x, y);   // 确保精确到位
    return true;
}

bool mouseDrag(int x1, int y1, int x2, int y2, const std::string& button, int durationMs) {
    SetCursorPos(x1, y1);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    DWORD down = (button == "right") ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
    DWORD up   = (button == "right") ? MOUSEEVENTF_RIGHTUP   : MOUSEEVENTF_LEFTUP;
    INPUT d = {}; d.type = INPUT_MOUSE; d.mi.dwFlags = down;
    SendInput(1, &d, sizeof(INPUT));
    mouseMoveSmooth(x2, y2, durationMs);
    INPUT u = {}; u.type = INPUT_MOUSE; u.mi.dwFlags = up;
    SendInput(1, &u, sizeof(INPUT));
    return true;
}

bool getMousePos(int& x, int& y) {
    POINT p;
    if (!GetCursorPos(&p)) return false;
    x = p.x; y = p.y;
    return true;
}

bool mouseClick(int x, int y, const std::string& button) {
    // 按下与抬起之间的保持时长（毫秒），可在设置中调整，默认 20ms
    const int holdMs = Settings::instance().inputClickHoldMs();
    auto hold = [holdMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(holdMs));
    };
    if (button == "right") {
        sendMouse(x, y, MOUSEEVENTF_RIGHTDOWN);
        hold();
        sendMouse(x, y, MOUSEEVENTF_RIGHTUP);
    } else if (button == "middle") {
        sendMouse(x, y, MOUSEEVENTF_MIDDLEDOWN);
        hold();
        sendMouse(x, y, MOUSEEVENTF_MIDDLEUP);
    } else if (button == "double") {
        sendMouse(x, y, MOUSEEVENTF_LEFTDOWN);
        hold();
        sendMouse(x, y, MOUSEEVENTF_LEFTUP);
        hold(); // 两次点击之间的间隔，取与保持时长相同的小间隙
        sendMouse(x, y, MOUSEEVENTF_LEFTDOWN);
        hold();
        sendMouse(x, y, MOUSEEVENTF_LEFTUP);
    } else {
        sendMouse(x, y, MOUSEEVENTF_LEFTDOWN);
        hold();
        sendMouse(x, y, MOUSEEVENTF_LEFTUP);
    }
    return true;
}

bool mouseWheel(int delta) {
    INPUT in = {};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_WHEEL;
    in.mi.mouseData = (DWORD)delta;
    SendInput(1, &in, sizeof(INPUT));
    return true;
}

// 单键映射
static WORD keyToVk(const std::string& key) {
    static std::map<std::string, WORD> m = {
        // 英文键名
        {"enter", VK_RETURN}, {"return", VK_RETURN}, {"tab", VK_TAB},
        {"space", VK_SPACE}, {" ", VK_SPACE},
        {"esc", VK_ESCAPE}, {"escape", VK_ESCAPE},
        {"backspace", VK_BACK}, {"delete", VK_DELETE}, {"del", VK_DELETE},
        {"up", VK_UP}, {"down", VK_DOWN}, {"left", VK_LEFT}, {"right", VK_RIGHT},
        {"home", VK_HOME}, {"end", VK_END}, {"pageup", VK_PRIOR}, {"pagedown", VK_NEXT},
        {"insert", VK_INSERT}, {"capslock", VK_CAPITAL},
        {"printscreen", VK_SNAPSHOT}, {"scrolllock", VK_SCROLL}, {"pause", VK_PAUSE},
        {"apps", VK_APPS}, {"menu", VK_APPS},
        {"f1", VK_F1}, {"f2", VK_F2}, {"f3", VK_F3}, {"f4", VK_F4},
        {"f5", VK_F5}, {"f6", VK_F6}, {"f7", VK_F7}, {"f8", VK_F8},
        {"f9", VK_F9}, {"f10", VK_F10}, {"f11", VK_F11}, {"f12", VK_F12},
        {"ctrl", VK_CONTROL}, {"control", VK_CONTROL}, {"shift", VK_SHIFT},
        {"alt", VK_MENU}, {"win", VK_LWIN}, {"windows", VK_LWIN},
        // 符号键
        {"-", VK_OEM_MINUS}, {"=", VK_OEM_PLUS}, {"[", VK_OEM_4}, {"]", VK_OEM_6},
        {"\\", VK_OEM_5}, {";", VK_OEM_1}, {"'", VK_OEM_7}, {",", VK_OEM_COMMA},
        {".", VK_OEM_PERIOD}, {"/", VK_OEM_2}, {"`", VK_OEM_3},
        // 中文键名
        {"空格", VK_SPACE}, {"回车", VK_RETURN}, {"换行", VK_RETURN}, {"制表", VK_TAB},
        {"退格", VK_BACK}, {"删除", VK_DELETE}, {"上", VK_UP}, {"下", VK_DOWN},
        {"左", VK_LEFT}, {"右", VK_RIGHT}, {"主页", VK_HOME}, {"结束", VK_END},
        {"上页", VK_PRIOR}, {"下页", VK_NEXT}, {"插入", VK_INSERT},
        {"大写锁定", VK_CAPITAL}, {"退出", VK_ESCAPE},
    };
    auto it = m.find(key);
    if (it != m.end()) return it->second;
    // 单个字符键
    std::string k = key;
    std::transform(k.begin(), k.end(), k.begin(), ::tolower);
    if (k.size() == 1 && k[0] >= 'a' && k[0] <= 'z') return (WORD)(k[0] - 'a' + 'A');
    if (k.size() == 1 && k[0] >= '0' && k[0] <= '9') return (WORD)k[0];
    return 0;
}

bool keyPress(const std::string& key) {
    std::string k = key;
    std::transform(k.begin(), k.end(), k.begin(), ::tolower);
    WORD vk = keyToVk(k);
    if (!vk) return false;
    INPUT in[2] = {};
    in[0].type = INPUT_KEYBOARD; in[0].ki.wVk = vk;
    in[1].type = INPUT_KEYBOARD; in[1].ki.wVk = vk; in[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, in, sizeof(INPUT));
    return true;
}

bool keyCombo(const std::string& keys) {
    auto parts = split(keys, '+');
    std::vector<WORD> mods;
    WORD mainKey = 0;
    for (auto& p : parts) {
        std::string k = trim(p);
        std::string kl = k;
        std::transform(kl.begin(), kl.end(), kl.begin(), ::tolower);
        WORD vk = keyToVk(kl);
        if (!vk) return false;
        if (kl == "ctrl" || kl == "control" || kl == "shift" || kl == "alt" || kl == "win" || kl == "windows")
            mods.push_back(vk);
        else
            mainKey = vk;
    }
    std::vector<INPUT> in;
    for (WORD m : mods) {
        INPUT i = {}; i.type = INPUT_KEYBOARD; i.ki.wVk = m;
        in.push_back(i);
    }
    if (mainKey) {
        INPUT i = {}; i.type = INPUT_KEYBOARD; i.ki.wVk = mainKey;
        in.push_back(i);
    }
    // 释放
    std::vector<INPUT> up;
    if (mainKey) { INPUT i = {}; i.type = INPUT_KEYBOARD; i.ki.wVk = mainKey; i.ki.dwFlags = KEYEVENTF_KEYUP; up.push_back(i); }
    for (auto it = mods.rbegin(); it != mods.rend(); ++it) {
        INPUT i = {}; i.type = INPUT_KEYBOARD; i.ki.wVk = *it; i.ki.dwFlags = KEYEVENTF_KEYUP;
        up.push_back(i);
    }
    SendInput((UINT)in.size(), in.data(), sizeof(INPUT));
    SendInput((UINT)up.size(), up.data(), sizeof(INPUT));
    return true;
}

bool keyType(const std::string& text) {
    std::wstring w;
    // UTF-8 -> UTF-16
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (len <= 1) return true;
    w.resize(len - 1);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &w[0], len);

    // 逐字符模拟 KEYEVENTF_UNICODE 按键（不使用剪贴板粘贴），
    // 每个字符发送后按设置的间隔休眠（默认 20ms），避免目标程序丢键。
    const int intervalMs = Settings::instance().inputTypeIntervalMs();
    for (wchar_t ch : w) {
        INPUT in[2] = {};
        in[0].type = INPUT_KEYBOARD;
        in[0].ki.wScan = ch; in[0].ki.dwFlags = KEYEVENTF_UNICODE;
        in[1].type = INPUT_KEYBOARD;
        in[1].ki.wScan = ch; in[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(2, in, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }
    return true;
}

bool setClipboardText(const std::string& text) {
    if (!OpenClipboard(nullptr)) return false;
    EmptyClipboard();
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    size_t bytes = (size_t)len * sizeof(wchar_t);
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!h) { CloseClipboard(); return false; }
    wchar_t* p = (wchar_t*)GlobalLock(h);
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, p, len);
    GlobalUnlock(h);
    SetClipboardData(CF_UNICODETEXT, h);
    CloseClipboard();
    return true;
}

std::string getClipboardText() {
    std::string out;
    if (!OpenClipboard(nullptr)) return out;
    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (h) {
        wchar_t* p = (wchar_t*)GlobalLock(h);
        if (p) {
            int len = WideCharToMultiByte(CP_UTF8, 0, p, -1, nullptr, 0, nullptr, nullptr);
            if (len > 1) {
                out.resize(len - 1);
                WideCharToMultiByte(CP_UTF8, 0, p, -1, &out[0], len, nullptr, nullptr);
            }
            GlobalUnlock(h);
        }
    }
    CloseClipboard();
    return out;
}

} // namespace InputSimulator
} // namespace autoflow
