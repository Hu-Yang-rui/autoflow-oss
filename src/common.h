#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <functional>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <mutex>

#include <nlohmann/json.hpp>
using nlohmann::json;

#include <QtCore/qglobal.h>   // QT_TRANSLATE_NOOP（仅标记，运行时恒等）

namespace autoflow {

// 指令五大分类（QT_TRANSLATE_NOOP 仅供 lupdate 提取；显示处在 "Instructions" 上下文翻译）
namespace Category {
    constexpr const char* Image = QT_TRANSLATE_NOOP("Instructions", "图像");
    constexpr const char* Input = QT_TRANSLATE_NOOP("Instructions", "键鼠");
    constexpr const char* Data  = QT_TRANSLATE_NOOP("Instructions", "数据");
    constexpr const char* Flow  = QT_TRANSLATE_NOOP("Instructions", "流程");
    constexpr const char* AI    = QT_TRANSLATE_NOOP("Instructions", "AI");
    constexpr const char* Window = QT_TRANSLATE_NOOP("Instructions", "窗口");
    constexpr const char* File   = QT_TRANSLATE_NOOP("Instructions", "文件");
    constexpr const char* System = QT_TRANSLATE_NOOP("Instructions", "系统");
}

// 出错处理策略
namespace OnError {
    constexpr const char* Abort = "abort"; // 中止
    constexpr const char* Skip  = "skip";  // 跳过
    constexpr const char* Retry = "retry"; // 重试
}

inline std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    std::istringstream ss(s);
    while (std::getline(ss, cur, delim)) out.push_back(cur);
    return out;
}

inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, b - a + 1);
}

// 去掉数字浮点尾随 0
inline std::string fmtNumber(double v) {
    if (v == (long long)v) return std::to_string((long long)v);
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

} // namespace autoflow
