#pragma once
#include "../common.h"
#include <QImage>
#include <vector>

namespace autoflow {

// 识别到的单个词及其边界框（坐标相对传入图片的像素，左上角原点）
struct OcrWordInfo {
    std::string text;
    int x = 0, y = 0, w = 0, h = 0;
};

// Windows 内置 OCR（Windows.Media.Ocr）：识别 img 中所有词及坐标。
// 零外部依赖、离线、免费；语言取决于系统已安装的 OCR 语言包。
// 识别失败时返回空并设置 err。
std::vector<OcrWordInfo> ocrWords(const QImage& img, std::string& err);

} // namespace autoflow
