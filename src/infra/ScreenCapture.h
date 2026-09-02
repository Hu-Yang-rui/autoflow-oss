#pragma once
#include "../common.h"

namespace autoflow {

// 屏幕抓取（Win32 BitBlt，配 Qt 保存 PNG 用于截图日志）
namespace ScreenCapture {

// 抓取整个屏幕（或指定区域）为 PNG，保存到 path。region 为空 = 全屏。
bool captureToPng(const std::string& path,
                  int x = 0, int y = 0, int w = 0, int h = 0);

// 抓取屏幕为内存中的图像字节（BMP/PNG），供找图/找色用（Phase 2 由 OpenCV 消费）
bool captureToFile(const std::string& path, int x = 0, int y = 0, int w = 0, int h = 0);

// 找色（纯 Win32，无需 OpenCV）：全屏扫描指定 RGB，返回首个匹配像素
struct ColorResult { bool found = false; int x = 0, y = 0; };
ColorResult findColor(int r, int g, int b, int tolerance);

// 截图前隐藏 AutoFlow 主窗口（纯 Win32 API，跨线程安全），
// 避免把工具窗口截进画面、干扰找图/OCR/AI 识别。返回是否成功隐藏。
bool hideSelfWindow();
// 恢复 AutoFlow 主窗口
void showSelfWindow();

} // namespace ScreenCapture
} // namespace autoflow
