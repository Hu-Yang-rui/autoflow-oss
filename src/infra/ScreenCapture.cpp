#include "ScreenCapture.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <QImage>
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>

namespace autoflow {
namespace ScreenCapture {

static int screenW() { return GetSystemMetrics(SM_CXSCREEN); }
static int screenH() { return GetSystemMetrics(SM_CYSCREEN); }

// 用 Win32 BitBlt 抓屏到 QImage
static bool grabToImage(QImage& img, int x, int y, int w, int h) {
    int sw = screenW(), sh = screenH();
    if (w <= 0) w = sw;
    if (h <= 0) h = sh;

    HDC hdcScreen = GetDC(nullptr);
    if (!hdcScreen) return false;
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, w, h);
    if (!hBitmap) { DeleteDC(hdcMem); ReleaseDC(nullptr, hdcScreen); return false; }

    HGDIOBJ old = SelectObject(hdcMem, hBitmap);
    BOOL ok = BitBlt(hdcMem, 0, 0, w, h, hdcScreen, x, y, SRCCOPY);
    SelectObject(hdcMem, old);

    if (ok) {
        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = w;
        bi.biHeight = -h;   // top-down
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;

        img = QImage(w, h, QImage::Format_RGB32);
        GetDIBits(hdcMem, hBitmap, 0, h, img.bits(),
                  (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    }
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    return ok != 0;
}

bool captureToPng(const std::string& path, int x, int y, int w, int h) {
    QImage img;
    if (!grabToImage(img, x, y, w, h)) return false;
    return img.save(QString::fromStdString(path), "PNG");
}

bool captureToFile(const std::string& path, int x, int y, int w, int h) {
    return captureToPng(path, x, y, w, h);
}

ColorResult findColor(int r, int g, int b, int tolerance) {
    ColorResult res;
    QImage img;
    if (!grabToImage(img, 0, 0, 0, 0)) return res;
    for (int y = 0; y < img.height(); ++y) {
        const QRgb* line = (const QRgb*)img.constScanLine(y);
        for (int x = 0; x < img.width(); ++x) {
            QRgb px = line[x];
            int pr = qRed(px), pg = qGreen(px), pb = qBlue(px);
            if (std::abs(pr - r) <= tolerance && std::abs(pg - g) <= tolerance &&
                std::abs(pb - b) <= tolerance) {
                res.found = true;
                res.x = x;
                res.y = y;
                return res;
            }
        }
    }
    return res;
}

// 查找 AutoFlow 主窗口（按 Qt 元对象类名）
static HWND findSelfWindow() {
    return FindWindowW(L"autoflow::MainWindow", nullptr);
}

bool hideSelfWindow() {
    HWND h = findSelfWindow();
    if (!h) return false;
    ShowWindow(h, SW_HIDE);
    Sleep(150);   // 等窗口隐藏消息处理

    // 等待桌面合成完成（DwmFlush），确保窗口区域已刷新为底层画面，
    // 避免 BitBlt 截到窗口残留。动态加载避免引入 dwmapi.lib 链接依赖。
    HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
    if (dwm) {
        using DwmFlushFn = HRESULT(WINAPI*)();
        auto flush = reinterpret_cast<DwmFlushFn>(GetProcAddress(dwm, "DwmFlush"));
        if (flush) { flush(); flush(); }   // 两帧，确保完全刷新
        FreeLibrary(dwm);
    }
    Sleep(50);
    return true;
}

void showSelfWindow() {
    HWND h = findSelfWindow();
    if (h) ShowWindow(h, SW_SHOW);
}

} // namespace ScreenCapture
} // namespace autoflow
