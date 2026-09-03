#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <QApplication>
#include <QIcon>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QFont>
#include <QFontMetrics>
#include <QFontInfo>
#include <QImage>
#include <QPainter>
#include <QLineEdit>
#include <QTableWidget>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QTreeWidget>
#include <QScreen>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QSpinBox>
#include <QLocale>
#include <QTranslator>
#include <QSettings>
#include "ui/FlowCanvas.h"
#include "ui/ParamPanel.h"
#include "ui/NodeItem.h"

#include "ui/MainWindow.h"
#include "ui/ThemeManager.h"
#include "ui/InstructionPanel.h"
#include "ui/SettingsDialog.h"
#include "ui/ThemeToggle.h"
#include "ui/TemplateDialog.h"
#include "core/ExecutionEngine.h"
#include "core/FlowModel.h"
#include "core/Settings.h"
#include "instructions/Builtins.h"

#include <fstream>
#include <string>

using namespace autoflow;

static void applySmileySansFont();   // 前置声明

// i18n：按 QSettings "general/language"（默认 zh）加载对应语言的翻译。
// 中文为源语言，无需 .qm；其他语言按语言代码加载 :/autoflow_<code>.qm。
// 新增语言：1) 生成 translations/autoflow_<code>.ts（源=中文，参考 autoflow_en.ts 结构）
//            2) lrelease 生成 resources/autoflow_<code>.qm，3) 加入 resources.qrc。
static void installTranslator() {
    const QString lang = QSettings("AutoFlow", "AutoFlow")
                             .value("general/language", "zh").toString();
    if (lang == "zh" || lang.isEmpty()) return;
    // 常驻堆对象（父对象为 app），生命周期覆盖整个运行期
    auto* translator = new QTranslator(QCoreApplication::instance());
    if (translator->load(":/autoflow_" + lang + ".qm"))
        QCoreApplication::installTranslator(translator);
    else
        delete translator;   // 无该语言 .qm 时回退中文源串
}
static QImage renderGlyph(const QFont& baseFont, const QString& text, int px);
static int pixelDiff(const QImage& a, const QImage& b);

// 无界面冒烟测试：加载示例流程，同步跑一遍，把结果写入文件（用于 CI / 验证）
static int runSmokeTest(const std::string& flowPath, const std::string& outPath) {
    registerBuiltinInstructions();

    std::string report;
    auto log = [&](const QString& lvl, const QString& id, const QString& name, const QString& txt) {
        report += "[" + lvl.toStdString() + "] " + name.toStdString() + "(" + id.toStdString() + "): "
                + txt.toStdString() + "\n";
    };

    EngineWorker worker;
    QObject::connect(&worker, &EngineWorker::logMessage,
                     [&](const QString& l, const QString& i, const QString& n, const QString& t) {
                         log(l, i, n, t);
                     });
    QObject::connect(&worker, &EngineWorker::nodeFinished,
                     [&](const QString& id, bool ok, qint64 ms, const QString& e) {
                         report += "    · 节点 " + id.toStdString()
                                 + (ok ? " 成功 " : " 失败 ") + std::to_string(ms) + "ms"
                                 + (ok ? "" : (" 错误=" + e.toStdString())) + "\n";
                     });

    bool finished = false;
    bool ok = false;
    QString summary;
    QObject::connect(&worker, &EngineWorker::runFinished,
                     [&](bool o, const QString& s) { finished = true; ok = o; summary = s; });

    FlowModel flow;
    std::string err;
    if (!flow.loadFromFile(flowPath, err)) {
        report += "加载流程失败: " + err + "\n";
    } else {
        report += "=== 流程: " + flow.name + " ===\n";
        worker.setFlow(flow);
        worker.runFlow(QString());
        report += "\n结果: " + std::string(ok ? "成功" : "失败") + " — "
                + summary.toStdString() + "\n";
    }

    std::ofstream f(outPath, std::ios::binary);
    f << report;
    f.close();
    (void)finished;
    return ok ? 0 : 1;
}

// 渲染文本并用像素回归测量字形倾斜度（>0 表示向右倾斜）
static double measureSlant(const QFont& baseFont, const QString& text) {
    QFont f = baseFont;
    f.setPointSize(80);
    QFontMetrics fm(f);
    QRect br = fm.tightBoundingRect(text);
    if (br.isEmpty()) return 0.0;
    QImage img(br.width() + 40, br.height() + 40, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setFont(f);
    p.setPen(Qt::black);
    p.drawText(QPoint(20 - br.left(), 20 - br.top()), text);
    p.end();

    double n = 0, sx = 0, sy = 0, sxy = 0, syy = 0;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            if (qAlpha(img.pixel(x, y)) > 128) {
                sx += x; sy += y; sxy += (double)x * y; syy += (double)y * y; n += 1;
            }
        }
    }
    if (n < 50) return 0.0;
    return (n * sxy - sx * sy) / (n * syy - sy * sy);   // x = a*y + b 的斜率 a
}

// 逐控件核查实际渲染字体，定位“哪些部分没覆盖”，结果写入文件
static int runWidgetFontCheck(const std::string& outPath) {
    registerBuiltinInstructions();
    applySmileySansFont();
    ThemeManager::instance().setDark(false);

    MainWindow w;
    w.show();                       // 显示以触发 QSS 完整应用
    QApplication::processEvents();

    std::string report;
    auto walk = [&](auto&& self, QWidget* root, int depth) -> void {
        const auto kids = root->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
        for (QWidget* child : kids) {
            QFontInfo fi(child->font());
            QString fams;
            for (const QString& ff : child->font().families())
                fams += ff + ";";
            report += std::string(depth * 2, ' ') + std::string(child->metaObject()->className())
                    + " |obj=" + (child->objectName().isEmpty() ? std::string("-") : child->objectName().toStdString())
                    + " |font=" + fi.family().toStdString()
                    + " |style=" + fi.styleName().toStdString()
                    + " |families=[" + fams.toStdString() + "]\n";
            self(self, child, depth + 1);
        }
    };
    walk(walk, &w, 0);

    // 用真实控件的最终字体（QSS 已应用）渲染数字，确认实际渲染来源
    if (QTableWidget* logTable = w.findChild<QTableWidget*>("logTable")) {
        QFont actual = logTable->font();
        report += "logTable 最终字体族: [" + actual.families().join(";").toStdString() + "]\n";
        QImage iFb = renderGlyph(actual, QString::fromUtf8("11"), 40);
        QImage iSeg = renderGlyph(QFont("Segoe UI"), QString::fromUtf8("11"), 40);
        QImage iSmi = renderGlyph(QFont("Smiley Sans Oblique"), QString::fromUtf8("11"), 40);
        report += "logTable '11' 渲染: vsSegoe=" + std::to_string(pixelDiff(iFb, iSeg))
                + " vsSmiley=" + std::to_string(pixelDiff(iFb, iSmi)) + "\n";
    }
    if (QLineEdit* le = w.findChild<QLineEdit*>()) {
        QFont actual = le->font();
        report += "QLineEdit 最终字体族: [" + actual.families().join(";").toStdString() + "]\n";
        QImage iFb = renderGlyph(actual, QString::fromUtf8("11"), 40);
        QImage iSeg = renderGlyph(QFont("Segoe UI"), QString::fromUtf8("11"), 40);
        QImage iSmi = renderGlyph(QFont("Smiley Sans Oblique"), QString::fromUtf8("11"), 40);
        report += "QLineEdit '11' 渲染: vsSegoe=" + std::to_string(pixelDiff(iFb, iSeg))
                + " vsSmiley=" + std::to_string(pixelDiff(iFb, iSmi)) + "\n";
    }
    // 画布场景有自己独立的字体（节点上的数字），单独核查
    if (QGraphicsView* view = w.findChild<QGraphicsView*>("flowCanvas")) {
        QGraphicsScene* scene = view->scene();
        if (scene) {
            QFont sceneFont = scene->font();
            report += "画布场景字体族: [" + sceneFont.families().join(";").toStdString() + "]\n";
            QImage iFb = renderGlyph(sceneFont, QString::fromUtf8("11"), 40);
            QImage iSeg = renderGlyph(QFont("Segoe UI"), QString::fromUtf8("11"), 40);
            QImage iSmi = renderGlyph(QFont("Smiley Sans Oblique"), QString::fromUtf8("11"), 40);
            report += "画布 '11' 渲染: vsSegoe=" + std::to_string(pixelDiff(iFb, iSeg))
                    + " vsSmiley=" + std::to_string(pixelDiff(iFb, iSmi)) + "\n";
        }
    }

    // 参数面板：选中一个带数字参数的节点，检查输入控件的数字渲染
    if (QGraphicsView* view = w.findChild<QGraphicsView*>("flowCanvas")) {
        if (auto* scene = qobject_cast<FlowCanvasScene*>(view->scene())) {
            scene->addNode("delay", QPointF(500, 500));
            NodeItem* last = nullptr;
            for (QGraphicsItem* it : scene->items()) {
                if (auto* ni = dynamic_cast<NodeItem*>(it)) last = ni;
            }
            if (last) {
                if (ParamPanel* pp = w.findChild<ParamPanel*>()) {
                    pp->setNode(last->nodeId());
                    QApplication::processEvents();
                    if (QSpinBox* spin = pp->findChild<QSpinBox*>()) {
                        QFont sf = spin->font();
                        std::string val = spin->text().toStdString();
                        report += "参数面板 QSpinBox 字体族: [" + sf.families().join(";").toStdString()
                                + "] 值=" + val + "\n";
                        QImage iFb = renderGlyph(sf, QString::fromStdString(val), 40);
                        QImage iSeg = renderGlyph(QFont("Segoe UI"), QString::fromStdString(val), 40);
                        QImage iSmi = renderGlyph(QFont("Smiley Sans Oblique"), QString::fromStdString(val), 40);
                        report += "参数面板数字 '" + val + "' 渲染: vsSegoe=" + std::to_string(pixelDiff(iFb, iSeg))
                                + " vsSmiley=" + std::to_string(pixelDiff(iFb, iSmi)) + "\n";
                    } else {
                        report += "参数面板未找到 QSpinBox\n";
                    }
                }
            }
        }
    }

    std::ofstream ofs(outPath, std::ios::binary);
    ofs << report;
    return 0;
}

// 渲染文本并返回位图（用于判断某字符实际由哪个字体渲染）
static QImage renderGlyph(const QFont& baseFont, const QString& text, int px) {
    QFont f = baseFont;
    f.setPointSize(px);
    QFontMetrics fm(f);
    QRect br = fm.tightBoundingRect(text);
    if (br.isEmpty()) return QImage();
    QImage img(br.width() + 40, br.height() + 40, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setFont(f);
    p.setPen(Qt::black);
    p.drawText(QPoint(20 - br.left(), 20 - br.top()), text);
    p.end();
    return img;
}

static int pixelDiff(const QImage& a, const QImage& b) {
    if (a.isNull() || b.isNull()) return -1;
    QImage ra = a.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QImage rb = b.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    int diff = 0;
    for (int y = 0; y < 48; ++y)
        for (int x = 0; x < 48; ++x)
            if (ra.pixel(x, y) != rb.pixel(x, y)) ++diff;
    return diff;
}

// 字体检查：确认 Smiley Sans 被 Qt 正确加载（族名 + 样式 + 字形覆盖），结果写入文件
static int runFontCheck(const std::string& outPath) {
    int id = QFontDatabase::addApplicationFont(":/fonts/SmileySans-Oblique.ttf");
    std::string report;
    report += "addApplicationFont id=" + std::to_string(id) + "\n";
    if (id >= 0) {
        const QStringList families = QFontDatabase::applicationFontFamilies(id);
        for (const QString& fam : families) {
            report += "family=" + fam.toStdString() + "\n";
            const QStringList styles = QFontDatabase::styles(fam);
            for (const QString& s : styles)
                report += "  style=" + s.toStdString() + "\n";
        }
        // 字形覆盖检查（O=得意黑包含, X=回退系统字体）
        QFont font("Smiley Sans Oblique");
        font.setPointSize(10);

        QFontInfo fi(font);
        report += "resolved family=" + fi.family().toStdString() + "\n";
        report += "resolved styleName=" + fi.styleName().toStdString() + "\n";
        report += "resolved italic=" + std::to_string(fi.italic()) + "\n";
        report += "resolved exactMatch=" + std::to_string(fi.exactMatch()) + "\n";

        QFontMetrics fm(font);
        auto cov = [&](const char* label, const QString& s) {
            report += std::string(label) + ": ";
            for (QChar c : s) { report += fm.inFont(c) ? "O" : "X"; report += " "; }
            report += "\n";
        };
        cov("digits  ", QString::fromUtf8("0123456789"));
        cov("latin   ", QString::fromUtf8("ABCXYZabcxyz"));
        cov("symbols ", QString::fromUtf8("\u25B6\u25A0\u23ED\u2192\u00B7\u00D7\u2460\u2461\u2462"));
        cov("punct   ", QString::fromUtf8("，。：；（）％℃＃《》【】！？"));
        cov("chinese ", QString::fromUtf8("可视化自动化鼠标点击运行停止"));

        // 倾斜度测量（正值=右倾）
        auto slant = [&](const char* label, const QString& s) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.4f", measureSlant(font, s));
            report += std::string("slant ") + label + "(" + s.toStdString() + ")=" + buf + "\n";
        };
        slant("0",    QString::fromUtf8("0"));
        slant("1",    QString::fromUtf8("1"));
        slant("O",    QString::fromUtf8("O"));
        slant("中",   QString::fromUtf8("中"));
        slant("回",   QString::fromUtf8("回"));

        // 实际 UI 字体的 fallback 行为：数字应走 Segoe UI(直立≈0)，中文走得意黑(斜)
        QFont uiFont = ThemeManager::smileySansFont();
        uiFont.setPointSize(10);
        auto slantUi = [&](const char* label, const QString& s) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.4f", measureSlant(uiFont, s));
            report += std::string("uiSlant ") + label + "(" + s.toStdString() + ")=" + buf + "\n";
        };
        slantUi("0", QString::fromUtf8("0"));
        slantUi("1", QString::fromUtf8("1"));
        slantUi("11", QString::fromUtf8("11"));
        slantUi("2", QString::fromUtf8("2"));
        slantUi("中", QString::fromUtf8("中"));

        // 像素对比：判断数字实际由哪个字体渲染（diff 越小越接近该字体）
        QFont seg("Segoe UI");
        QFont smi("Smiley Sans Oblique");
        QFont fallback = ThemeManager::smileySansFont();
        for (const char* ch : {"0", "1", "2", "11"}) {
            QImage iSeg = renderGlyph(seg, QString::fromUtf8(ch), 40);
            QImage iSmi = renderGlyph(smi, QString::fromUtf8(ch), 40);
            QImage iFb  = renderGlyph(fallback, QString::fromUtf8(ch), 40);
            int dSeg = pixelDiff(iFb, iSeg);
            int dSmi = pixelDiff(iFb, iSmi);
            report += std::string("glyph '") + ch + "': vsSegoe=" + std::to_string(dSeg)
                    + " vsSmiley=" + std::to_string(dSmi)
                    + " -> " + (dSeg <= dSmi ? "Segoe UI" : "Smiley Sans") + "\n";
        }
    }
    std::ofstream ofs(outPath, std::ios::binary);
    ofs << report;
    return id >= 0 ? 0 : 1;
}

// 应用系统默认字体（不内嵌字体）：基础字号 10pt，叠加 Settings 字号缩放
static void applySmileySansFont() {
    QFont f = ThemeManager::smileySansFont();
    // 基础字号 10pt，叠加 Settings 的字号缩放（百分比，默认 100 = 行为不变）
    f.setPointSizeF(10.0 * Settings::instance().fontScale() / 100.0);
    QApplication::setFont(f);
    // 条目视图(QAbstractItemView)使用独立字体角色，需单独设置
    QApplication::setFont(f, "QAbstractItemView");
}

int main(int argc, char** argv) {
    // 声明 DPI 感知（系统级）：让 Win32 GetSystemMetrics/BitBlt 拿到物理分辨率。
    // 否则 4K 屏（如 150% 缩放）会被 DPI 虚拟化缩到 2560x1440，截图模糊、AI 识别细节丢失。
    // 必须在 QApplication 构造前调用。
    SetProcessDPIAware();

    // 冒烟测试模式（无 GUI）：AutoFlow.exe --smoke <流程.json> <输出.txt>
    if (argc >= 4 && std::string(argv[1]) == "--smoke") {
        QCoreApplication app(argc, argv);
        return runSmokeTest(argv[2], argv[3]);
    }

    // 字体检查模式：AutoFlow.exe --font-check <输出.txt>
    if (argc >= 3 && std::string(argv[1]) == "--font-check") {
        QApplication app(argc, argv);
        return runFontCheck(argv[2]);
    }

    // 逐控件字体核查：AutoFlow.exe --widget-font-check <输出.txt>
    if (argc >= 3 && std::string(argv[1]) == "--widget-font-check") {
        QApplication app(argc, argv);
        return runWidgetFontCheck(argv[2]);
    }

    // 渲染诊断：AutoFlow.exe --render-tree <输出.png> [dark] [focus]
    if (argc >= 3 && std::string(argv[1]) == "--render-tree") {
        QApplication app(argc, argv);
        installTranslator();
        registerBuiltinInstructions();
        applySmileySansFont();
        bool dark = (argc >= 4 && std::string(argv[3]) == "dark");
        bool forceFocus = (argc >= 5 && std::string(argv[4]) == "focus");
        MainWindow w;
        w.resize(1440, 860);
        ThemeManager::instance().setDark(dark);
        w.show();
        QApplication::processEvents();
        // 用真实鼠标事件点击第一个指令项，重现用户点击后的完整状态（含焦点框）
        if (auto* tree = w.findChild<QTreeWidget*>("instructionPanel")) {
            if (forceFocus) {
                // 强制给 viewport 焦点，重现真实点击后的焦点状态（验证 delegate 是否去掉虚线框）
                tree->viewport()->setFocusPolicy(Qt::StrongFocus);
                tree->viewport()->setFocus();
            }
            for (int i = 0; i < tree->topLevelItemCount(); ++i) {
                QTreeWidgetItem* cat = tree->topLevelItem(i);
                if (cat->childCount() > 0) {
                    QTreeWidgetItem* item = cat->child(0);
                    tree->scrollToItem(item);
                    QApplication::processEvents();
                    QRect r = tree->visualItemRect(item);
                    QPoint c = r.center();
                    QMouseEvent press(QEvent::MouseButtonPress, c,
                                      tree->viewport()->mapToGlobal(c),
                                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                    QMouseEvent release(QEvent::MouseButtonRelease, c,
                                        tree->viewport()->mapToGlobal(c),
                                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                    QApplication::sendEvent(tree->viewport(), &press);
                    QApplication::sendEvent(tree->viewport(), &release);
                    // 可选 hover 参数：模拟鼠标移到第二个指令项（未选中），验证纯 hover 态（灰色边框）
                    if (argc >= 5 && std::string(argv[4]) == "hover") {
                        QTreeWidgetItem* hoverItem = cat->childCount() > 1 ? cat->child(1) : cat->child(0);
                        QRect hr = tree->visualItemRect(hoverItem);
                        QPoint hc = hr.center();
                        QCursor::setPos(tree->viewport()->mapToGlobal(hc));
                        QHoverEvent he(QEvent::HoverMove, QPointF(hc), QPointF(hc), QPointF(hc));
                        QApplication::sendEvent(tree->viewport(), &he);
                        QApplication::processEvents();
                    }
                    break;
                }
            }
        }
        if (forceFocus) {
            w.activateWindow();
            QApplication::processEvents();
            if (auto* tree = w.findChild<QTreeWidget*>("instructionPanel")) {
                tree->viewport()->setFocusPolicy(Qt::StrongFocus);
                tree->viewport()->setFocus();
            }
            QApplication::processEvents();
        }
        QImage img = w.grab().toImage();
        img.save(QString::fromLocal8Bit(argv[2]));
        return 0;
    }

    // 渲染设置对话框：AutoFlow.exe --render-settings <输出.png> [dark]
    if (argc >= 3 && std::string(argv[1]) == "--render-settings") {
        QApplication app(argc, argv);
        installTranslator();
        registerBuiltinInstructions();
        applySmileySansFont();
        bool dark = (argc >= 4 && std::string(argv[3]) == "dark");
        ThemeManager::instance().setDark(dark);
        ThemeToggle toggle;
        SettingsDialog dlg(&toggle);
        dlg.resize(760, 560);
        dlg.show();
        QApplication::processEvents();
        QImage img = dlg.grab().toImage();
        img.save(QString::fromLocal8Bit(argv[2]));
        return 0;
    }

    // 渲染模板对话框：AutoFlow.exe --render-template <输出.png> [dark]
    if (argc >= 3 && std::string(argv[1]) == "--render-template") {
        QApplication app(argc, argv);
        installTranslator();
        registerBuiltinInstructions();
        applySmileySansFont();
        bool dark = (argc >= 4 && std::string(argv[3]) == "dark");
        ThemeManager::instance().setDark(dark);
        QVector<TemplateItem> items = {
            { QStringLiteral("找图并点击"), QStringLiteral("在屏幕上找到模板图片后自动点击其位置"), QStringLiteral("开始 → 找图 → 结束") },
            { QStringLiteral("定时点击"), QStringLiteral("延时指定时间后在固定坐标点击"), QStringLiteral("开始 → 延时 → 点击 → 结束") },
            { QStringLiteral("等待画面"), QStringLiteral("循环等待目标图片出现，超时则失败"), QStringLiteral("开始 → 等待画面 → 结束") },
            { QStringLiteral("循环点击"), QStringLiteral("重复点击指定次数"), QStringLiteral("开始 → 循环(点击) → 结束") },
            { QStringLiteral("AI 监控点击"), QStringLiteral("每秒用 AI 识别目标，识别到后自动点击其位置"), QStringLiteral("开始 → 循环(AI识别→点击→延时) → 结束") },
        };
        TemplateDialog dlg(items);
        dlg.resize(400, 340);
        dlg.show();
        QApplication::processEvents();
        QImage img = dlg.grab().toImage();
        img.save(QString::fromLocal8Bit(argv[2]));
        return 0;
    }

    // 字体探测：AutoFlow.exe --font-probe <输出.txt>
    if (argc >= 3 && std::string(argv[1]) == "--font-probe") {
        QApplication app(argc, argv);
        applySmileySansFont();
        std::string r;
        auto probe = [&](const char* name) {
            QFont f(QString::fromUtf8(name));
            QFontInfo fi(f);
            r += std::string(name) + " -> family=" + fi.family().toStdString()
               + " weight=" + std::to_string(fi.weight())
               + " pixelSize=" + std::to_string(fi.pixelSize())
               + " exact=" + std::to_string(fi.exactMatch()) + "\n";
        };
        probe("Microsoft YaHei UI");
        probe("Microsoft YaHei UI Light");
        probe("Microsoft YaHei");
        probe("Noto Sans SC");
        probe("Noto Sans SC Light");
        auto probeW = [&](const char* name, int w) {
            QFont f(QString::fromUtf8(name)); f.setWeight((QFont::Weight)w);
            QFontInfo fi(f);
            r += std::string(name) + " w=" + std::to_string(w) + " -> family=" + fi.family().toStdString()
               + " weight=" + std::to_string(fi.weight()) + " exact=" + std::to_string(fi.exactMatch()) + "\n";
        };
        probeW("Microsoft YaHei", QFont::Light);
        probeW("Microsoft YaHei", QFont::Normal);
        probeW("Noto Sans SC", QFont::Light);
        probeW("Noto Sans SC", QFont::Normal);
        QFontInfo def(app.font());
        r += "app font -> family=" + def.family().toStdString() + " weight=" + std::to_string(def.weight()) + "\n";
        std::ofstream of(argv[2]);
        of << r;
        return 0;
    }

    QApplication app(argc, argv);
    QApplication::setApplicationName("AutoFlow");
    QApplication::setOrganizationName("AutoFlow");
    installTranslator();
    QApplication::setApplicationDisplayName(
        QCoreApplication::translate("AutoFlow", "AutoFlow 可视化自动化工具"));
    app.setWindowIcon(QIcon(":/app_icon.png"));

    // 强制 ASCII 数字显示，避免中文系统把数字显示成「〇一二三/〡〢〣」等中文数字
    QLocale::setDefault(QLocale::c());

    registerBuiltinInstructions();
    applySmileySansFont();
    ThemeManager::instance().setDark(true);

    // 静默启动（--silent）：不显示主窗口，进入静默运行模式（缩到托盘，交互指令自动跳过）
    bool silentStart = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--silent") { silentStart = true; break; }
    }
    if (silentStart) Settings::instance().setRunSilent(true);

    MainWindow w;
    w.resize(1440, 860);
    if (!silentStart) {
        w.show();
        // 恢复上次保存的窗口几何；无记录时保持 1440x860 默认
        const QByteArray geo = QSettings("AutoFlow", "AutoFlow").value("window/geometry").toByteArray();
        if (!geo.isEmpty()) w.restoreGeometry(geo);
    }
    return app.exec();
}
