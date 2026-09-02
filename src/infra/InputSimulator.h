#pragma once
#include "../common.h"

namespace autoflow {

// 键鼠模拟（Windows API: SendInput / SetCursorPos）
namespace InputSimulator {

bool mouseMove(int x, int y);
bool mouseMoveSmooth(int x, int y, int durationMs);                 // 平滑移动（durationMs<=0 时瞬移）
bool mouseClick(int x, int y, const std::string& button = "left");   // left|right|middle|double
bool mouseDrag(int x1, int y1, int x2, int y2,                       // 从起点按住拖到终点
               const std::string& button = "left", int durationMs = 200);
bool getMousePos(int& x, int& y);                                    // 读取当前鼠标坐标
bool mouseWheel(int delta);                                          // 正=向上
bool keyType(const std::string& text);                               // 键入文本（支持中文）
bool keyCombo(const std::string& keys);                              // 如 "ctrl+c"、"alt+tab"
bool keyPress(const std::string& key);                               // 单键，如 "enter"、"f5"

bool setClipboardText(const std::string& text);
std::string getClipboardText();

} // namespace InputSimulator
} // namespace autoflow
