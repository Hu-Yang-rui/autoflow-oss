#include "IInstruction.h"
#include "InstructionRegistry.h"
#include "../infra/ScreenCapture.h"
#include "../infra/InfraStubs.h"
#include "../infra/InputSimulator.h"
#include "../infra/HttpClient.h"
#include "../infra/OcrWin.h"
#include "../core/Settings.h"

#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileInfoList>
#include <QImage>

#include <thread>
#include <chrono>
#include <cmath>
#include <fstream>
#include <regex>

namespace autoflow {

// ============================ 图像 / AI 类指令 ============================

static std::string tempScreenshotPath() {
    QString dir = Settings::instance().shotDir();
    if (dir.isEmpty()) dir = QDir::tempPath() + "/autoflow";
    QDir().mkpath(dir);
    if (Settings::instance().shotAutoClean()) {
        // 清理 1 小时前的旧截图（仅 shot_*.png，避免误删）
        qint64 cutoff = QDateTime::currentMSecsSinceEpoch() - 3600LL * 1000;
        const QFileInfoList old = QDir(dir).entryInfoList({ "shot_*.png" }, QDir::Files);
        for (const QFileInfo& fi : old)
            if (fi.lastModified().toMSecsSinceEpoch() < cutoff)
                QFile::remove(fi.absoluteFilePath());
    }
    QString name = "shot_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    return (dir + "/" + name).toStdString();
}

struct FindImageInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "findimage"; m.category = Category::Image; m.name = QT_TRANSLATE_NOOP("Instructions", "找图");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "在屏幕上查找目标图片（模板匹配）");
        m.params = {
            Param("template", QT_TRANSLATE_NOOP("Instructions", "模板图片路径"), "string", "assets/target.png",
                  QT_TRANSLATE_NOOP("Instructions", "PNG/JPG 图片路径")),
            Param("threshold", QT_TRANSLATE_NOOP("Instructions", "相似度阈值"), "number", "0.85",
                  QT_TRANSLATE_NOOP("Instructions", "0~1，越大越严格")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果保存到变量"), "string", "match",
                  QT_TRANSLATE_NOOP("Instructions", "对象：found/x/y/score")),
            Param("clickIfFound", QT_TRANSLATE_NOOP("Instructions", "找到后点击"), "bool", "false",
                  QT_TRANSLATE_NOOP("Instructions", "点击模板中心点"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string tpl = ctx.pStr(params, "template");
        double threshold = ctx.pNum(params, "threshold", 0.85);
        std::string saveVar = ctx.pStr(params, "saveVar", "match");
        bool click = ctx.pBool(params, "clickIfFound");

        std::string shot = tempScreenshotPath();
        ScreenCapture::hideSelfWindow();
        bool shotOk = ScreenCapture::captureToPng(shot);
        ScreenCapture::showSelfWindow();
        if (!shotOk) { ctx.error = QCoreApplication::translate("Instructions", "屏幕抓取失败").toStdString(); return ""; }

        MatchResult r = imageMatch(shot, tpl, threshold);
        if (!r.found) {
            ctx.vars.set(saveVar, Variable::makeObject());
            if (ctx.notifyVar) ctx.notifyVar(saveVar);
            ctx.error = r.error.empty()
                ? QCoreApplication::translate("Instructions", "未找到目标图片").toStdString() : r.error;
            return "";
        }
        Variable v = Variable::makeObject();
        v.object["found"] = Variable::makeBool(true);
        v.object["x"] = Variable::makeNumber(r.x);
        v.object["y"] = Variable::makeNumber(r.y);
        v.object["score"] = Variable::makeNumber(r.score);
        ctx.vars.set(saveVar, v);
        if (ctx.notifyVar) ctx.notifyVar(saveVar);

        ctx.info(QCoreApplication::translate("Instructions", "找到目标图片，位置 (%1, %2)，相似度 %3")
                     .arg(r.x).arg(r.y)
                     .arg(QString::fromStdString(fmtNumber(r.score))).toStdString());
        if (click) InputSimulator::mouseClick(r.x + 10, r.y + 10);
        return "next";
    }
};

struct FindColorInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "findcolor"; m.category = Category::Image; m.name = QT_TRANSLATE_NOOP("Instructions", "找色");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "在屏幕上查找指定颜色（无需 OpenCV）");
        m.params = {
            Param("r", QT_TRANSLATE_NOOP("Instructions", "R 分量"), "int", "255", "0~255"),
            Param("g", QT_TRANSLATE_NOOP("Instructions", "G 分量"), "int", "0", "0~255"),
            Param("b", QT_TRANSLATE_NOOP("Instructions", "B 分量"), "int", "0", "0~255"),
            Param("tolerance", QT_TRANSLATE_NOOP("Instructions", "容差"), "int", "10",
                  QT_TRANSLATE_NOOP("Instructions", "允许的颜色误差")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果变量"), "string", "colorMatch",
                  QT_TRANSLATE_NOOP("Instructions", "对象：found/x/y")),
            Param("clickIfFound", QT_TRANSLATE_NOOP("Instructions", "找到后点击"), "bool", "false", "")
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int r = ctx.pInt(params, "r", 255);
        int g = ctx.pInt(params, "g", 0);
        int b = ctx.pInt(params, "b", 0);
        int tol = ctx.pInt(params, "tolerance", 10);
        std::string saveVar = ctx.pStr(params, "saveVar", "colorMatch");
        bool click = ctx.pBool(params, "clickIfFound");

        ScreenCapture::hideSelfWindow();
        ScreenCapture::ColorResult res = ScreenCapture::findColor(r, g, b, tol);
        ScreenCapture::showSelfWindow();
        if (!res.found) {
            ctx.vars.set(saveVar, Variable::makeObject());
            if (ctx.notifyVar) ctx.notifyVar(saveVar);
            ctx.error = QCoreApplication::translate("Instructions", "未找到指定颜色").toStdString();
            return "";
        }
        Variable v = Variable::makeObject();
        v.object["found"] = Variable::makeBool(true);
        v.object["x"] = Variable::makeNumber(res.x);
        v.object["y"] = Variable::makeNumber(res.y);
        ctx.vars.set(saveVar, v);
        if (ctx.notifyVar) ctx.notifyVar(saveVar);

        ctx.info(QCoreApplication::translate("Instructions", "找到颜色，位置 (%1, %2)")
                     .arg(res.x).arg(res.y).toStdString());
        if (click) InputSimulator::mouseClick(res.x, res.y);
        return "next";
    }
};

struct WaitImageInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "waitimage"; m.category = Category::Image; m.name = QT_TRANSLATE_NOOP("Instructions", "等待画面");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "循环等待某张图片出现，超时则失败");
        m.params = {
            Param("template", QT_TRANSLATE_NOOP("Instructions", "模板图片路径"), "string", "assets/target.png", ""),
            Param("threshold", QT_TRANSLATE_NOOP("Instructions", "相似度阈值"), "number", "0.85", ""),
            Param("timeout", QT_TRANSLATE_NOOP("Instructions", "超时(毫秒)"), "int", "10000",
                  QT_TRANSLATE_NOOP("Instructions", "最长等待时间")),
            Param("interval", QT_TRANSLATE_NOOP("Instructions", "轮询间隔(毫秒)"), "int", "500",
                  QT_TRANSLATE_NOOP("Instructions", "每隔多久检查一次")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果变量"), "string", "match", "")
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string tpl = ctx.pStr(params, "template");
        double threshold = ctx.pNum(params, "threshold", 0.85);
        int timeout = ctx.pInt(params, "timeout", 10000);
        int interval = ctx.pInt(params, "interval", 500);
        std::string saveVar = ctx.pStr(params, "saveVar", "match");

        auto t0 = std::chrono::steady_clock::now();
        ScreenCapture::hideSelfWindow();
        while (true) {
            std::string shot = tempScreenshotPath();
            ScreenCapture::captureToPng(shot);
            MatchResult r = imageMatch(shot, tpl, threshold);
            if (r.found) {
                Variable v = Variable::makeObject();
                v.object["found"] = Variable::makeBool(true);
                v.object["x"] = Variable::makeNumber(r.x);
                v.object["y"] = Variable::makeNumber(r.y);
                ctx.vars.set(saveVar, v);
                if (ctx.notifyVar) ctx.notifyVar(saveVar);
                ScreenCapture::showSelfWindow();
                ctx.info(QCoreApplication::translate("Instructions", "等待到目标画面出现").toStdString());
                return "next";
            }
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now() - t0).count();
            if (elapsed >= timeout) { ScreenCapture::showSelfWindow(); ctx.error = QCoreApplication::translate("Instructions", "等待超时，画面未出现").toStdString(); return ""; }
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }
};

struct OcrInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "ocr"; m.category = Category::Image; m.name = QT_TRANSLATE_NOOP("Instructions", "OCR 识别");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "识别屏幕或图片中的文字，可定位并点击指定文字");
        m.params = {
            Param("capture", QT_TRANSLATE_NOOP("Instructions", "截图后识别"), "bool", "true",
                  QT_TRANSLATE_NOOP("Instructions", "开启后自动截取当前屏幕识别，无需填图片路径")),
            Param("image", QT_TRANSLATE_NOOP("Instructions", "图片路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "仅在关闭「截图后识别」时识别该图片")).opt(),
            Param("findText", QT_TRANSLATE_NOOP("Instructions", "要找的文字"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "填了则定位该文字并返回坐标；留空则识别全部文字")),
            Param("clickIfFound", QT_TRANSLATE_NOOP("Instructions", "找到后点击"), "bool", "false",
                  QT_TRANSLATE_NOOP("Instructions", "定位到文字后点击其中心")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "文本保存到变量"), "string", "ocrText",
                  QT_TRANSLATE_NOOP("Instructions", "填「要找的文字」时存对象 found/x/y/text"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        const std::string findText = ctx.pStr(params, "findText");
        const bool click = ctx.pBool(params, "clickIfFound");
        const std::string saveVar = ctx.pStr(params, "saveVar", "ocrText");

        // 截图 / 读图（向后兼容：老流程无 capture 字段时，image 留空即截图）
        const bool capture = params.contains("capture")
            ? ctx.pBool(params, "capture", true)
            : ctx.pStr(params, "image").empty();
        const std::string image = ctx.pStr(params, "image");
        QString path;
        if (capture) {
            path = QString::fromStdString(tempScreenshotPath());
            ScreenCapture::hideSelfWindow();
            const bool ok = ScreenCapture::captureToPng(path.toStdString());
            ScreenCapture::showSelfWindow();
            if (!ok) { ctx.error = QCoreApplication::translate("Instructions", "屏幕抓取失败").toStdString(); return ""; }
        } else {
            if (image.empty()) {
                ctx.error = QCoreApplication::translate("Instructions", "请填写图片路径，或开启「截图后识别」").toStdString();
                return "";
            }
            path = QString::fromStdString(image);
        }

        QImage img(path);
        if (img.isNull()) {
            ctx.error = QCoreApplication::translate("Instructions", "无法读取图片: %1")
                            .arg(QString::fromStdString(image)).toStdString();
            return "";
        }

        // 直接用原图识别（不缩放），避免缩小后小字模糊导致识别失败
        const double scale = 1.0;

        std::string err;
        const std::vector<OcrWordInfo> words = ocrWords(img, err);
        if (!err.empty()) { ctx.error = err; return ""; }

        // 未填「要找的文字」：识别全部文字（按行拼接）
        if (findText.empty()) {
            std::string fullText;
            int lastY = -1;
            for (const OcrWordInfo& w : words) {
                if (!fullText.empty()) {
                    const int lineH = w.h > 0 ? w.h : 12;
                    if (lastY >= 0 && std::abs(w.y - lastY) > lineH) fullText += "\n";
                    else fullText += " ";
                }
                fullText += w.text;
                lastY = w.y;
            }
            ctx.vars.set(saveVar, Variable::makeString(fullText));
            if (ctx.notifyVar) ctx.notifyVar(saveVar);
            ctx.info(QCoreApplication::translate("Instructions", "OCR 识别完成: %1")
                         .arg(QString::fromStdString(fullText)).toStdString());
            return "next";
        }

        // 定位指定文字：先单词匹配（英文单词/中文单字），再按行拼接匹配（中文多字词被拆成单字）
        bool found = false;
        int cx = 0, cy = 0;
        std::string matched;
        for (const OcrWordInfo& w : words) {
            if (w.text.find(findText) != std::string::npos) {
                found = true;
                cx = (int)std::lround((w.x + w.w / 2.0) * scale);
                cy = (int)std::lround((w.y + w.h / 2.0) * scale);
                matched = w.text;
                break;
            }
        }
        if (!found) {
            size_t i = 0;
            while (i < words.size()) {
                // 收集同一行（y 坐标接近）的词，按顺序拼接（不加空格，兼容中文按字分词）
                const int baseY = words[i].y;
                const int lineH = words[i].h > 0 ? words[i].h : 12;
                std::string lineText;
                std::vector<size_t> idxs;
                while (i < words.size() && std::abs(words[i].y - baseY) <= lineH) {
                    lineText += words[i].text;
                    idxs.push_back(i);
                    ++i;
                }
                const size_t pos = lineText.find(findText);
                if (pos != std::string::npos) {
                    int minX = 1000000000, maxX = -1000000000, minY = 1000000000, maxY = -1000000000;
                    size_t acc = 0;
                    for (size_t k : idxs) {
                        const OcrWordInfo& w = words[k];
                        const size_t s = acc, e = acc + w.text.size();
                        acc = e;
                        if (s < pos + findText.size() && e > pos) {   // 该词被匹配区间覆盖
                            minX = std::min(minX, w.x); maxX = std::max(maxX, w.x + w.w);
                            minY = std::min(minY, w.y); maxY = std::max(maxY, w.y + w.h);
                        }
                    }
                    if (maxX != -1000000000) {
                        found = true;
                        cx = (int)std::lround((minX + maxX) / 2.0 * scale);
                        cy = (int)std::lround((minY + maxY) / 2.0 * scale);
                        matched = findText;
                    }
                    break;
                }
            }
        }

        Variable v = Variable::makeObject();
        v.object["found"] = Variable::makeBool(found);
        v.object["x"] = Variable::makeNumber(cx);
        v.object["y"] = Variable::makeNumber(cy);
        v.object["text"] = Variable::makeString(found ? matched : findText);
        ctx.vars.set(saveVar, v);
        if (ctx.notifyVar) ctx.notifyVar(saveVar);

        if (!found) {
            std::string fullText;
            for (const OcrWordInfo& w : words) fullText += w.text;
            ctx.error = QCoreApplication::translate("Instructions", "未找到文字「%1」，实际识别到：%2")
                            .arg(QString::fromStdString(findText))
                            .arg(QString::fromStdString(fullText.substr(0, 120))).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "OCR 找到「%1」于 (%2, %3)")
                     .arg(QString::fromStdString(findText)).arg(cx).arg(cy).toStdString());
        if (click) InputSimulator::mouseClick(cx, cy);
        return "next";
    }
};

// base64 编码（内联实现，无外部依赖）
static std::string base64Encode(const std::string& data) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve((data.size() + 2) / 3 * 4);
    size_t i = 0;
    for (; i + 3 <= data.size(); i += 3) {
        unsigned v = ((unsigned char)data[i] << 16) | ((unsigned char)data[i + 1] << 8) | (unsigned char)data[i + 2];
        out += tbl[(v >> 18) & 63]; out += tbl[(v >> 12) & 63];
        out += tbl[(v >> 6) & 63];  out += tbl[v & 63];
    }
    size_t rem = data.size() - i;
    if (rem == 1) {
        unsigned v = (unsigned char)data[i] << 16;
        out += tbl[(v >> 18) & 63]; out += tbl[(v >> 12) & 63];
        out += '='; out += '=';
    } else if (rem == 2) {
        unsigned v = ((unsigned char)data[i] << 16) | ((unsigned char)data[i + 1] << 8);
        out += tbl[(v >> 18) & 63]; out += tbl[(v >> 12) & 63];
        out += tbl[(v >> 6) & 63];  out += '=';
    }
    return out;
}

// 读取图片实际宽高（用于归一化坐标换算），失败返回 false
static bool readImageSize(const std::string& path, int& w, int& h) {
    QImage img(QString::fromStdString(path));
    if (img.isNull()) return false;
    w = img.width();
    h = img.height();
    return w > 0 && h > 0;
}

// 归一化数值 → 像素：0~1000（v/1000*dim）或 0~1（v*dim），并 clamp 到 [0, dim-1]
static int normToPixel(double v, int dim) {
    if (dim <= 0) return (int)std::lround(v); // 无尺寸信息时按像素原样返回
    double px = (v > 1.0) ? (v / 1000.0 * dim) : (v * dim);
    int r = (int)std::lround(px);
    if (r < 0) r = 0;
    if (r > dim - 1) r = dim - 1;
    return r;
}

// 两个角点取中心：normalized 时按归一化换算，否则按像素
static void boxCenter(double x1, double y1, double x2, double y2,
                      bool normalized, int imgW, int imgH, int& x, int& y) {
    if (normalized) {
        x = normToPixel((x1 + x2) / 2.0, imgW);
        y = normToPixel((y1 + y2) / 2.0, imgH);
    } else {
        x = (int)std::lround((x1 + x2) / 2.0);
        y = (int)std::lround((y1 + y2) / 2.0);
    }
}

// 安全地把正则匹配组转成 double，解析失败返回 false（避免 stod 抛异常导致引擎崩溃）
static bool toDouble(const std::smatch& m, size_t i, double& out) {
    try {
        std::size_t pos = 0;
        out = std::stod(m[i].str(), &pos);
        return true;
    } catch (...) {
        return false;
    }
}

// 解析 AI 回答中的坐标：
//   A. 像素点：x=500,y=300、x:500 y:300、(500,300)（默认按像素）
//   B. Qwen-VL 归一化 box：<|box_start|>(x1,y1),(x2,y2)<|box_end|>（0~1000，取中心）
//   C. 方括号 box：[x1,y1,x2,y2]（取中心）
//   D. 双点 box：[[x1,y1],[x2,y2]]（取中心）
//   E. 标签 box：<box>x1 y1 x2 y2</box> 或 <x1 y1 x2 y2>（取中心）
//   F. 归一化单点：文本含 "归一化"/"normalized" 时按 0~1000 / 0~1 换算
static bool parseCoord(const std::string& text, int& x, int& y, int imgW, int imgH) {
    std::smatch m;
    double x1, y1, x2, y2;

    // 是否明确声明归一化
    const bool wantNorm = text.find("归一化") != std::string::npos
        || text.find("normalized") != std::string::npos
        || text.find("Normalized") != std::string::npos;

    // B. Qwen-VL 归一化 box
    {
        std::regex re(R"(<\|\s*box_start\s*\|\s*>\s*\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)\s*,\s*\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)\s*<\|\s*box_end\s*\|\s*>)");
        if (std::regex_search(text, m, re) && toDouble(m, 1, x1) && toDouble(m, 2, y1)
            && toDouble(m, 3, x2) && toDouble(m, 4, y2)) {
            // box 内出现 >1000 的数值视为像素坐标，否则按 0~1000 归一化
            bool asPixel = (x1 > 1000.0 || y1 > 1000.0 || x2 > 1000.0 || y2 > 1000.0);
            boxCenter(x1, y1, x2, y2, !asPixel, imgW, imgH, x, y);
            return true;
        }
    }

    // D. 双点 box：[[x1,y1],[x2,y2]]
    {
        std::regex re(R"(\[\s*\[\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\]\s*,\s*\[\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\]\s*\])");
        if (std::regex_search(text, m, re) && toDouble(m, 1, x1) && toDouble(m, 2, y1)
            && toDouble(m, 3, x2) && toDouble(m, 4, y2)) {
            bool asPixel = (x1 > 1000.0 || y1 > 1000.0 || x2 > 1000.0 || y2 > 1000.0);
            boxCenter(x1, y1, x2, y2, !asPixel, imgW, imgH, x, y);
            return true;
        }
    }

    // C. 方括号 box：[x1,y1,x2,y2]
    {
        std::regex re(R"(\[\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\])");
        if (std::regex_search(text, m, re) && toDouble(m, 1, x1) && toDouble(m, 2, y1)
            && toDouble(m, 3, x2) && toDouble(m, 4, y2)) {
            bool asPixel = (x1 > 1000.0 || y1 > 1000.0 || x2 > 1000.0 || y2 > 1000.0);
            boxCenter(x1, y1, x2, y2, !asPixel, imgW, imgH, x, y);
            return true;
        }
    }

    // E. 标签 box：<box>x1 y1 x2 y2</box>
    {
        std::regex re(R"(<\s*box\s*>\s*(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s*<\s*/\s*box\s*>)");
        if (std::regex_search(text, m, re) && toDouble(m, 1, x1) && toDouble(m, 2, y1)
            && toDouble(m, 3, x2) && toDouble(m, 4, y2)) {
            bool asPixel = (x1 > 1000.0 || y1 > 1000.0 || x2 > 1000.0 || y2 > 1000.0);
            boxCenter(x1, y1, x2, y2, !asPixel, imgW, imgH, x, y);
            return true;
        }
    }

    // E'. 标签 box 简写：<x1 y1 x2 y2>
    {
        std::regex re(R"(<\s*(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s*>)");
        if (std::regex_search(text, m, re) && toDouble(m, 1, x1) && toDouble(m, 2, y1)
            && toDouble(m, 3, x2) && toDouble(m, 4, y2)) {
            bool asPixel = (x1 > 1000.0 || y1 > 1000.0 || x2 > 1000.0 || y2 > 1000.0);
            boxCenter(x1, y1, x2, y2, !asPixel, imgW, imgH, x, y);
            return true;
        }
    }

    // A. 像素点 / 归一化单点：x=500,y=300、x:500 y:300
    {
        std::regex re(R"([xX]\s*[=:]\s*(-?\d+(?:\.\d+)?).*?[yY]\s*[=:]\s*(-?\d+(?:\.\d+)?))");
        if (std::regex_search(text, m, re) && toDouble(m, 1, x1) && toDouble(m, 2, y1)) {
            if (wantNorm) { x = normToPixel(x1, imgW); y = normToPixel(y1, imgH); }
            else { x = (int)std::lround(x1); y = (int)std::lround(y1); }
            return true;
        }
    }

    // A'. 像素点 / 归一化单点：(500,300)
    {
        std::regex re(R"(\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\))");
        if (std::regex_search(text, m, re) && toDouble(m, 1, x1) && toDouble(m, 2, y1)) {
            if (wantNorm) { x = normToPixel(x1, imgW); y = normToPixel(y1, imgH); }
            else { x = (int)std::lround(x1); y = (int)std::lround(y1); }
            return true;
        }
    }

    return false;
}

// 解析 AI 服务端点：provider 预设回填 + 补全 /chat/completions。
// 供主服务与备选方案复用；providerIn 为空时回退智谱预设（兼容旧配置）。
static bool resolveAiEndpoint(const QString& providerIn, const QString& baseUrlIn,
                              const QString& modelIn, QString& baseUrl, QString& model,
                              std::string& err) {
    QString provider = providerIn;
    baseUrl = baseUrlIn;
    model = modelIn;
    if (provider.isEmpty()) provider = "zhipu";
    if (provider == "zhipu") {
        if (baseUrl.isEmpty()) baseUrl = "https://open.bigmodel.cn/api/paas/v4/chat/completions";
        if (model.isEmpty()) model = "glm-4-flash";
    } else if (provider == "ollama") {
        if (baseUrl.isEmpty()) baseUrl = "http://localhost:11434/v1/chat/completions";
        if (model.isEmpty()) model = "llama3.2-vision";
        // 用户可能只填服务器根地址（如 http://localhost:11434），自动补全 OpenAI 兼容端点
        if (!baseUrl.contains("chat/completions")) {
            if (!baseUrl.endsWith('/')) baseUrl += '/';
            baseUrl += "v1/chat/completions";
        }
    } else if (provider == "lmstudio") {
        if (baseUrl.isEmpty()) baseUrl = "http://localhost:1234/v1/chat/completions";
        if (model.isEmpty()) model = "local-model";
        if (!baseUrl.contains("chat/completions")) {
            if (!baseUrl.endsWith('/')) baseUrl += '/';
            baseUrl += "v1/chat/completions";
        }
    }
    if (baseUrl.isEmpty()) {
        err = QCoreApplication::translate("Instructions", "未配置 AI 服务端点，请在 设置→AI 服务 中填写 Base URL").toStdString();
        return false;
    }
    // 自动补全 /chat/completions：用户可能只填基础端点（如 https://api.a6api.com/v1）
    if (!baseUrl.contains("chat/completions", Qt::CaseInsensitive)) {
        if (!baseUrl.endsWith('/')) baseUrl += '/';
        baseUrl += "chat/completions";
    }
    if (model.isEmpty()) {
        err = QCoreApplication::translate("Instructions", "未配置 AI 模型，请在 设置→AI 服务 中填写模型名称").toStdString();
        return false;
    }
    return true;
}

// 对单个已解析端点执行一次 OpenAI 兼容 chat/completions 请求，返回 choices[0].message.content
static std::string callAiChatOnce(const QString& provider, const QString& baseUrl,
                                  const QString& model, const QString& apiKey,
                                  const json& messages, std::string& err) {
    QString p = provider;
    if (p.isEmpty()) p = "zhipu";
    const bool local = (p == "ollama" || p == "lmstudio");
    // 本地模型（Ollama / LM Studio）无需 API Key；云端服务才要求
    if (apiKey.isEmpty() && !local) {
        err = QCoreApplication::translate("Instructions", "未配置 AI 服务，请在 设置→AI 服务 中填写 API Key").toStdString();
        return "";
    }

    json body = { { "model", model.toStdString() }, { "messages", messages }, { "max_tokens", 512 } };
    std::map<std::string, std::string> headers;
    if (!apiKey.isEmpty()) headers["Authorization"] = "Bearer " + apiKey.toStdString();
    // AI 调用用固定 2 分钟超时：本地模型生成慢（含推理思考），全局 httpTimeoutMs(默认10s) 会提前断开
    HttpResponse res = HttpClient::request("POST", baseUrl.toStdString(), body.dump(),
                                           "application/json",
                                           120000, headers);
    if (!res.ok) {
        err = res.error.empty()
            ? QCoreApplication::translate("Instructions", "AI 服务请求失败").toStdString() : res.error;
        return "";
    }
    try {
        json r = json::parse(res.body);
        if (!r.contains("choices") || !r["choices"].is_array() || r["choices"].empty()
            || !r["choices"][0].contains("message")) {
            err = QCoreApplication::translate("Instructions", "AI 响应格式异常: %1")
                      .arg(QString::fromStdString(res.body.substr(0, 300))).toStdString();
            return "";
        }
        return r["choices"][0]["message"]["content"].get<std::string>();
    } catch (const std::exception&) {
        err = QCoreApplication::translate("Instructions", "AI 响应解析失败: %1")
                  .arg(QString::fromStdString(res.body.substr(0, 300))).toStdString();
        return "";
    }
}

// 调用 AI chat/completions：先走主服务，失败且启用备选方案时自动切换重试一次
static std::string callAiChat(const json& messages, std::string& err) {
    Settings& s = Settings::instance();

    // 主服务
    QString baseUrl, model;
    if (!resolveAiEndpoint(s.aiProvider(), s.aiBaseUrl(), s.aiModel(), baseUrl, model, err))
        return "";
    std::string primaryErr;
    std::string content = callAiChatOnce(s.aiProvider(), baseUrl, model, s.aiApiKey(), messages, primaryErr);
    if (primaryErr.empty()) {
        err.clear();
        return content;
    }

    // 主服务失败且启用备选方案：自动切换
    if (!s.aiBackupEnabled()) {
        err = primaryErr;
        return "";
    }
    QString bBaseUrl, bModel;
    std::string bResolveErr;
    if (!resolveAiEndpoint(s.aiBackupProvider(), s.aiBackupBaseUrl(), s.aiBackupModel(),
                           bBaseUrl, bModel, bResolveErr)) {
        err = QCoreApplication::translate("Instructions", "主服务失败（%1），备选方案未配置：%2")
                  .arg(QString::fromStdString(primaryErr)).arg(QString::fromStdString(bResolveErr)).toStdString();
        return "";
    }
    std::string backupErr;
    std::string backupContent = callAiChatOnce(s.aiBackupProvider(), bBaseUrl, bModel,
                                               s.aiBackupApiKey(), messages, backupErr);
    if (backupErr.empty()) {
        err.clear();
        return backupContent;
    }
    err = QCoreApplication::translate("Instructions", "主服务失败（%1），已尝试备选方案也失败：%2")
              .arg(QString::fromStdString(primaryErr)).arg(QString::fromStdString(backupErr)).toStdString();
    return "";
}

struct AiVisionInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "ai_vision"; m.category = Category::AI; m.name = QT_TRANSLATE_NOOP("Instructions", "AI 图像理解");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "调用 AI 服务理解图像内容，可返回目标坐标并点击");
        m.params = {
            Param("capture", QT_TRANSLATE_NOOP("Instructions", "截图后识别"), "bool", "true",
                  QT_TRANSLATE_NOOP("Instructions", "开启后自动截取当前屏幕识别，无需填图片路径")),
            Param("image", QT_TRANSLATE_NOOP("Instructions", "图片路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "仅在关闭「截图后识别」时识别该图片")).opt(),
            Param("prompt", QT_TRANSLATE_NOOP("Instructions", "提问"), "string", "描述这张图片的内容", ""),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果变量"), "string", "aiResult",
                  QT_TRANSLATE_NOOP("Instructions", "对象：found/x/y/text")),
            Param("clickIfFound", QT_TRANSLATE_NOOP("Instructions", "找到后点击"), "bool", "false",
                  QT_TRANSLATE_NOOP("Instructions", "AI 返回坐标后点击该位置"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string prompt = ctx.pStr(params, "prompt");
        bool click = ctx.pBool(params, "clickIfFound");
        std::string saveVar = ctx.pStr(params, "saveVar", "aiResult");

        // 向后兼容：老流程无 capture 字段时，image 留空即视为「截图后识别」
        const bool capture = params.contains("capture")
            ? ctx.pBool(params, "capture", true)
            : ctx.pStr(params, "image").empty();
        std::string image = ctx.pStr(params, "image");
        if (capture) {
            image = tempScreenshotPath();
            ScreenCapture::hideSelfWindow();
            bool shotOk = ScreenCapture::captureToPng(image);
            ScreenCapture::showSelfWindow();
            if (!shotOk) {
                ctx.error = QCoreApplication::translate("Instructions", "屏幕抓取失败").toStdString();
                return "";
            }
        } else if (image.empty()) {
            ctx.error = QCoreApplication::translate("Instructions", "请填写图片路径，或开启「截图后识别」").toStdString();
            return "";
        }
        std::ifstream f(image, std::ios::binary);
        if (!f) {
            ctx.error = QCoreApplication::translate("Instructions", "无法读取图片: %1")
                            .arg(QString::fromStdString(image)).toStdString();
            return "";
        }
        std::string data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        // 读取图片实际宽高，用于把归一化坐标换算成像素坐标；失败时按像素处理
        int imgW = 0, imgH = 0;
        readImageSize(image, imgW, imgH);

        // 在提问后追加坐标格式要求：让 AI 在找到目标时返回中心坐标
        std::string fullPrompt = prompt + "\n\n" + QCoreApplication::translate("Instructions",
            "这是一张屏幕截图，图片尺寸就是原始屏幕分辨率，坐标原点在左上角 (0,0)，x 向右增大、y 向下增大。"
            "请仔细在原图中定位目标，输出它的中心点坐标。"
            "坐标可用像素坐标 x=数字,y=数字（例如 x=500,y=300），"
            "也可用 Qwen-VL 风格归一化 box（坐标范围 0~1000）：<|box_start|>(x1,y1),(x2,y2)<|box_end|>，"
            "程序会取 box 中心点并按图片宽高换算成像素坐标。"
            "不要输出任何解释文字。"
            "如果画面中确实找不到该目标，只输出 notfound").toStdString();

        json messages = json::array({
            { { "role", "user" },
              { "content", json::array({
                  { { "type", "text" }, { "text", fullPrompt } },
                  { { "type", "image_url" },
                    { "image_url", { { "url", "data:image/png;base64," + base64Encode(data) } } } }
              }) } }
        });

        std::string err;
        std::string content = callAiChat(messages, err);
        if (!err.empty()) { ctx.error = err; return ""; }

        // 解析 AI 返回的坐标
        int x = -1, y = -1;
        const bool found = parseCoord(content, x, y, imgW, imgH);

        // saveVar 存对象：found / x / y / text
        Variable v = Variable::makeObject();
        v.object["found"] = Variable::makeBool(found);
        if (found) {
            v.object["x"] = Variable::makeNumber(x);
            v.object["y"] = Variable::makeNumber(y);
        }
        v.object["text"] = Variable::makeString(content);
        ctx.vars.set(saveVar, v);
        if (ctx.notifyVar) ctx.notifyVar(saveVar);

        if (found) {
            ctx.info(QCoreApplication::translate("Instructions", "AI 识别到目标，位置 (%1, %2)")
                         .arg(x).arg(y).toStdString());
            if (click) InputSimulator::mouseClick(x, y);
        } else {
            ctx.info(QCoreApplication::translate("Instructions", "AI 未识别到目标").toStdString());
        }
        return "next";
    }
};

struct AiExtractInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "ai_extract"; m.category = Category::AI; m.name = QT_TRANSLATE_NOOP("Instructions", "页面信息抽取");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "从文本/页面中抽取结构化信息");
        m.params = {
            Param("input", QT_TRANSLATE_NOOP("Instructions", "输入文本"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("schema", QT_TRANSLATE_NOOP("Instructions", "抽取字段(逗号分隔)"), "string", "标题,价格", ""),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果变量"), "string", "extracted", "")
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string input = ctx.pStr(params, "input");
        std::string schema = ctx.pStr(params, "schema");
        std::string saveVar = ctx.pStr(params, "saveVar", "extracted");

        std::string content = input;
        if (!schema.empty())
            content += "\n\n" + QCoreApplication::translate("Instructions",
                "请从上述内容中抽取以下字段，以 JSON 对象返回: ").toStdString() + schema;

        json messages = json::array({
            { { "role", "user" }, { "content", content } }
        });

        std::string err;
        std::string result = callAiChat(messages, err);
        if (!err.empty()) { ctx.error = err; return ""; }
        ctx.vars.set(saveVar, Variable::makeString(result));
        if (ctx.notifyVar) ctx.notifyVar(saveVar);
        ctx.info(QCoreApplication::translate("Instructions", "页面信息抽取完成（%1 字节）")
                     .arg((qulonglong)result.size()).toStdString());
        return "next";
    }
};

void registerVisionInstructions() {
    registerInstruction(std::make_unique<FindImageInstr>());
    registerInstruction(std::make_unique<FindColorInstr>());
    registerInstruction(std::make_unique<WaitImageInstr>());
    registerInstruction(std::make_unique<OcrInstr>());
    registerInstruction(std::make_unique<AiVisionInstr>());
    registerInstruction(std::make_unique<AiExtractInstr>());
}

} // namespace autoflow
