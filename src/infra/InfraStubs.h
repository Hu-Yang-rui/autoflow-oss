#pragma once
#include "../common.h"
#include "../core/Settings.h"
#include <vector>
#include <QByteArray>
#include <QCoreApplication>
#include <QString>

// 可选能力的统一接口层：OpenCV / Tesseract / libxlsxwriter
// 未编译对应依赖时，返回明确的错误信息（Phase 2/3 按需开启 CMake 开关）

#ifdef AUTOPLOW_HAS_OPENCV
#include <opencv2/opencv.hpp>
#endif

namespace autoflow {

struct MatchResult {
    bool found = false;
    double score = 0.0;
    int x = 0, y = 0;        // 匹配点（左上角）
    std::string error;
};

// 找图：模板匹配（OpenCV）
inline MatchResult imageMatch(const std::string& haystackPng, const std::string& needlePng,
                              double threshold) {
    MatchResult r;
#ifdef AUTOPLOW_HAS_OPENCV
    cv::Mat hay = cv::imread(haystackPng, cv::IMREAD_COLOR);
    cv::Mat needle = cv::imread(needlePng, cv::IMREAD_COLOR);
    if (hay.empty() || needle.empty()) {
        r.error = QCoreApplication::translate("Infra", "无法读取图片（请检查路径）").toStdString();
        return r;
    }
    if (needle.cols > hay.cols || needle.rows > hay.rows) {
        r.error = QCoreApplication::translate("Infra", "模板图比目标图大").toStdString();
        return r;
    }
    cv::Mat res;
    cv::matchTemplate(hay, needle, res, cv::TM_CCOEFF_NORMED);
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc);
    r.score = maxVal;
    if (maxVal >= threshold) { r.found = true; r.x = maxLoc.x; r.y = maxLoc.y; }
    else r.error = QCoreApplication::translate("Infra", "未找到匹配图像（最高相似度 %1 < %2）")
                       .arg(QString::number(maxVal, 'f', 6))    // 与 std::to_string(double) 输出一致
                       .arg(QString::number(threshold, 'f', 6)).toStdString();
#else
    r.error = QCoreApplication::translate("Infra", "找图功能需要 OpenCV（cmake -DAUTOPLOW_WITH_OPENCV=ON）").toStdString();
#endif
    return r;
}

// OCR 识别（Tesseract）
inline std::string ocrImage(const std::string& pngPath, std::string& err) {
#ifdef AUTOPLOW_HAS_TESSERACT
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
    tesseract::TessBaseAPI api;
    QByteArray ocrLang = Settings::instance().ocrLanguage().toUtf8();  // 默认 "chi_sim+eng"
    if (api.Init(nullptr, ocrLang.constData())) {
        err = QCoreApplication::translate("Infra", "OCR 初始化失败（检查语言包）").toStdString();
        return "";
    }
    Pix* pix = pixRead(pngPath.c_str());
    if (!pix) {
        err = QCoreApplication::translate("Infra", "无法读取图片: %1")
                  .arg(QString::fromStdString(pngPath)).toStdString();
        return "";
    }
    api.SetImage(pix);
    char* text = api.GetUTF8Text();
    std::string result = text ? text : "";
    delete[] text;
    pixDestroy(&pix);
    api.End();
    return result;
#else
    err = QCoreApplication::translate("Infra", "OCR 需要 Tesseract（cmake -DAUTOPLOW_WITH_TESSERACT=ON）").toStdString();
    return "";
#endif
}

// Excel 写入（libxlsxwriter）
inline bool excelWriteRows(const std::string& xlsxPath, const std::string& sheetName,
                           const std::vector<std::vector<std::string>>& rows, std::string& err) {
#ifdef AUTOPLOW_HAS_XLSXWRITER
#include <xlsxwriter.h>
    lxw_workbook* wb = workbook_new(xlsxPath.c_str());
    if (!wb) {
        err = QCoreApplication::translate("Infra", "无法创建 Excel 文件: %1")
                  .arg(QString::fromStdString(xlsxPath)).toStdString();
        return false;
    }
    lxw_worksheet* ws = workbook_add_worksheet(wb, sheetName.empty() ? "Sheet1" : sheetName.c_str());
    for (size_t r = 0; r < rows.size(); ++r) {
        for (size_t c = 0; c < rows[r].size(); ++c)
            worksheet_write_string(ws, (lxw_row_t)r, (lxw_col_t)c, rows[r][c].c_str(), nullptr);
    }
    lxw_error e = workbook_close(wb);
    if (e != LXW_NO_ERROR) {
        err = QCoreApplication::translate("Infra", "Excel 保存失败（错误码 %1）")
                  .arg((int)e).toStdString();
        return false;
    }
    return true;
#else
    err = QCoreApplication::translate("Infra", "Excel 写入需要 libxlsxwriter（cmake -DAUTOPLOW_WITH_XLSXWRITER=ON）").toStdString();
    return false;
#endif
}

} // namespace autoflow
