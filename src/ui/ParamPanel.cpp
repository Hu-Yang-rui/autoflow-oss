#include "ParamPanel.h"
#include "Stepper.h"
#include "CropOverlay.h"
#include "Palette.h"
#include "ThemeManager.h"
#include "../instructions/InstructionRegistry.h"
#include "../infra/ScreenCapture.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QFrame>
#include <QScrollBar>
#include <QLocale>
#include <QEventLoop>
#include <QPixmap>
#include <QImage>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <QApplication>
#include <QRegularExpression>
#include <QStyle>

namespace autoflow {

namespace {
// select 选项显示层翻译：代码/英文键 → 中文（值仍存原值，仅显示翻译）
QString selectDisplayText(const std::string& option) {
    static const QHash<QString, QString> zh = {
        {QStringLiteral("left"),   QStringLiteral("左键")},
        {QStringLiteral("right"),  QStringLiteral("右键")},
        {QStringLiteral("middle"), QStringLiteral("中键")},
        {QStringLiteral("double"), QStringLiteral("双击")},
        {QStringLiteral("auto"),   QStringLiteral("自动")},
        {QStringLiteral("string"), QStringLiteral("字符串")},
        {QStringLiteral("number"), QStringLiteral("数字")},
        {QStringLiteral("bool"),   QStringLiteral("布尔")},
        {QStringLiteral("list"),   QStringLiteral("列表")},
        {QStringLiteral("object"), QStringLiteral("对象")},
    };
    const QString s = QString::fromStdString(option);
    const auto it = zh.constFind(s);
    return it != zh.constEnd() ? it.value() : s;
}

// 从 hint 解析数值范围（如 "0~255"、"0~1，越大越严格"）
bool parseRangeFromHint(const std::string& hint, double& min, double& max) {
    if (hint.empty()) return false;
    static const QRegularExpression re(
        QStringLiteral("(-?\\d+(?:\\.\\d+)?)\\s*~\\s*(-?\\d+(?:\\.\\d+)?)"));
    const auto m = re.match(QString::fromStdString(hint));
    if (!m.hasMatch()) return false;
    bool ok1 = false, ok2 = false;
    min = m.captured(1).toDouble(&ok1);
    max = m.captured(2).toDouble(&ok2);
    return ok1 && ok2 && min <= max;
}
} // namespace

ParamPanel::ParamPanel(FlowModel* model, QWidget* parent)
    : QWidget(parent), m_model(model) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    m_scroll = new QScrollArea(this);
    m_scroll->setObjectName("paramScroll");
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outer->addWidget(m_scroll);

    auto* frame = new QFrame();
    frame->setObjectName("paramPanel");
    m_scroll->setWidget(frame);
    m_content = frame;
}

void ParamPanel::setNode(const QString& nodeId) {
    m_nodeId = nodeId;
    m_instr = nullptr;
    if (FlowNode* n = m_model->nodeById(nodeId.toStdString()))
        m_instr = InstructionRegistry::instance().get(n->instr);
    rebuild();
}

void ParamPanel::clearNode() {
    m_nodeId.clear();
    m_instr = nullptr;
    rebuild();
}

void ParamPanel::rebuild() {
    m_updating = true;
    m_widgets.clear();
    m_labels.clear();
    m_errors.clear();
    m_paramMeta.clear();

    // 完全重建内容区：销毁旧控件，避免残留控件与新控件重叠（旧实现只删布局不删控件）
    if (QWidget* old = m_scroll->takeWidget())
        delete old;
    m_content = new QFrame();
    m_content->setObjectName("paramPanel");
    m_scroll->setWidget(m_content);

    auto* lay = new QVBoxLayout(m_content);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    m_title = new QLabel();
    m_title->setObjectName("paramNodeName");
    m_title->setWordWrap(true);
    lay->addWidget(m_title);

    m_desc = new QLabel();
    m_desc->setObjectName("paramNodeDesc");
    m_desc->setWordWrap(true);
    lay->addWidget(m_desc);

    if (!m_instr) {
        m_title->setText(tr("未选择步骤"));
        m_desc->setText(tr("在画布中点击一个步骤节点，或从左侧拖入新指令。"));
        lay->addStretch();
        m_updating = false;
        return;
    }

    auto meta = m_instr->meta();
    // 指令名/描述/参数标签/提示：源串保持中文 std::string，显示时翻译
    m_title->setText(trInstr(meta.name.c_str()));
    m_desc->setText(trInstr(meta.desc.c_str()));

    for (auto& p : meta.params)
        addParamRow(p, lay);

    // 通用高级设置
    auto* sep = new QLabel(tr("出错处理"));
    sep->setObjectName("sectionHeader");
    lay->addWidget(sep);

    auto* errLabel = new QLabel(tr("失败策略"));
    errLabel->setObjectName("fieldLabel");
    lay->addWidget(errLabel);
    m_onError = new QComboBox();
    m_onError->addItem(tr("中止流程"), "abort");
    m_onError->addItem(tr("跳过并继续"), "skip");
    m_onError->addItem(tr("重试后继续"), "retry");
    lay->addWidget(m_onError);

    auto* retryLabel = new QLabel(tr("重试次数"));
    retryLabel->setObjectName("fieldLabel");
    lay->addWidget(retryLabel);
    m_retry = new Stepper();
    m_retry->setRange(0, 100);
    m_retry->setDecimals(0);
    m_retry->setSingleStep(1);
    lay->addWidget(m_retry);

    auto* commentLabel = new QLabel(tr("备注"));
    commentLabel->setObjectName("fieldLabel");
    lay->addWidget(commentLabel);
    m_comment = new QLineEdit();
    m_comment->setPlaceholderText(tr("给这个步骤写个备注（可选）"));
    lay->addWidget(m_comment);

    lay->addStretch();

    // 回填当前值
    FlowNode* n = m_model->nodeById(m_nodeId.toStdString());
    if (n) {
        for (auto& p : meta.params) {
            QWidget* w = m_widgets.value(QString::fromStdString(p.key));
            if (!w) continue;
            json val = n->params.contains(p.key) ? n->params[p.key] : json();
            setParamWidget(w, p.type, val, p.def);
        }
        int idx = m_onError->findData(QString::fromStdString(n->onError));
        m_onError->setCurrentIndex(idx < 0 ? 0 : idx);
        m_retry->setValue(n->retry);
        m_comment->setText(QString::fromStdString(n->comment));
    }

    // 连接信号
    for (auto it = m_widgets.begin(); it != m_widgets.end(); ++it) {
        QString key = it.key();
        QWidget* w = it.value();
        if (auto* le = qobject_cast<QLineEdit*>(w))
            connect(le, &QLineEdit::textEdited, this, [this, key] { writeBack(key); });
        else if (auto* st = qobject_cast<Stepper*>(w)) {
            connect(st, &Stepper::valueChanged, this, [this, key] { writeBack(key); });
            connect(st, &Stepper::rangeErrorChanged, this, [this, key](bool on) {
                setParamError(key, on, on ? tr("数值超出范围") : QString());
            });
        }
        else if (auto* cb = qobject_cast<QComboBox*>(w))
            connect(cb, &QComboBox::currentIndexChanged, this, [this, key] { writeBack(key); });
        else if (auto* ch = qobject_cast<QCheckBox*>(w))
            connect(ch, &QCheckBox::toggled, this, [this, key] { writeBack(key); });
        else if (auto* te = qobject_cast<QPlainTextEdit*>(w))
            connect(te, &QPlainTextEdit::textChanged, this, [this, key] { writeBack(key); });
    }
    connect(m_onError, &QComboBox::currentIndexChanged, this, [this] {
        if (m_updating) return;
        if (FlowNode* n = m_model->nodeById(m_nodeId.toStdString())) {
            n->onError = m_onError->currentData().toString().toStdString();
            emit paramEdited(m_nodeId);   // 与其它参数同一写回链路：置脏 + 入撤销栈
        }
    });
    connect(m_retry, &Stepper::valueChanged, this, [this](double v) {
        if (m_updating) return;
        if (FlowNode* n = m_model->nodeById(m_nodeId.toStdString())) {
            n->retry = (int)v;
            emit paramEdited(m_nodeId);
        }
    });
    connect(m_comment, &QLineEdit::textEdited, this, [this](const QString& t) {
        if (FlowNode* n = m_model->nodeById(m_nodeId.toStdString())) {
            n->comment = t.toStdString();
            emit paramEdited(m_nodeId);
        }
    });

    // 条件判断（if）快捷预设联动：禁用无关的参数框 + 动态改「比较的值」标签
    if (m_instr && m_instr->meta().id == "if") {
        if (auto* presetBox = qobject_cast<QComboBox*>(m_widgets.value("preset"))) {
            auto updateIfPreset = [this] {
                auto* pb = qobject_cast<QComboBox*>(m_widgets.value("preset"));
                if (!pb) return;
                const QString preset = pb->currentText();
                const bool isCustom = (preset == "自定义");
                const bool needsRight = (preset == "OCR 识别到文字"
                                         || preset == "软件已打开" || preset == "软件未打开");
                if (QWidget* left = m_widgets.value("left")) left->setEnabled(isCustom);
                if (QWidget* op = m_widgets.value("op")) op->setEnabled(isCustom);
                if (QWidget* right = m_widgets.value("right")) right->setEnabled(isCustom || needsRight);
                if (QLabel* rl = m_labels.value("right")) {
                    QString lbl = tr("比较的值");
                    if (preset == "软件已打开" || preset == "软件未打开") lbl = tr("进程名");
                    else if (preset == "OCR 识别到文字") lbl = tr("要找的文字");
                    rl->setText(labelText(lbl, m_paramMeta.value("right").required));
                }
            };
            connect(presetBox, &QComboBox::currentIndexChanged, this,
                    [updateIfPreset](int) { updateIfPreset(); });
            updateIfPreset();
        }
    }

    // OCR / AI 图像理解：「截图后识别」开关联动「图片路径」输入框
    if (m_instr) {
        const std::string iid = m_instr->meta().id;
        if (iid == "ocr" || iid == "ai_vision") {
            if (auto* capBox = qobject_cast<QCheckBox*>(m_widgets.value("capture"))) {
                auto updateCapture = [this] {
                    auto* cb = qobject_cast<QCheckBox*>(m_widgets.value("capture"));
                    if (!cb) return;
                    const bool on = cb->isChecked();
                    if (QWidget* img = m_widgets.value("image")) img->setEnabled(!on);
                    if (QLabel* il = m_labels.value("image")) il->setEnabled(!on);
                };
                connect(capBox, &QCheckBox::toggled, this,
                        [updateCapture](bool) { updateCapture(); });
                updateCapture();
            }
        }
    }

    m_updating = false;
}

void ParamPanel::addParamRow(const IInstruction::Param& p, QVBoxLayout* lay) {
    m_paramMeta.insert(QString::fromStdString(p.key), p);

    auto* label = new QLabel(labelText(trInstr(p.label.c_str()), p.required));
    label->setObjectName("fieldLabel");
    label->setTextFormat(Qt::RichText);
    lay->addWidget(label);
    m_labels.insert(QString::fromStdString(p.key), label);

    QWidget* w = nullptr;
    QWidget* container = nullptr; // 模板图片参数：QLineEdit + “截图选取”按钮的行容器
    if (p.type == "int") {
        auto* st = new Stepper();
        st->setDecimals(0);
        st->setSingleStep(1);
        double mn = -1000000.0, mx = 1000000.0;
        parseRangeFromHint(p.hint, mn, mx);
        st->setRange(mn, mx);
        w = st;
    } else if (p.type == "number") {
        auto* st = new Stepper();
        st->setDecimals(4);
        st->setSingleStep(0.1);
        double mn = -1000000.0, mx = 1000000.0;
        parseRangeFromHint(p.hint, mn, mx);
        st->setRange(mn, mx);
        w = st;
    } else if (p.type == "bool") {
        // 复选框文字不再固定“启用”：语义由上方 label 表达
        auto* ch = new QCheckBox();
        ch->setText(QString());
        w = ch;
    } else if (p.type == "select") {
        auto* cb = new QComboBox();
        cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);   // 选项文字完整显示，不被截断
        for (auto& o : p.options)
            cb->addItem(selectDisplayText(o), QString::fromStdString(o));
        w = cb;
    } else if (p.type == "textarea") {
        auto* te = new QPlainTextEdit();
        te->setMaximumHeight(140);
        te->setMinimumHeight(70);
        w = te;
    } else if (p.type == "node") {
        auto* cb = new QComboBox();
        cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);   // 节点选项文字完整显示
        cb->addItem(tr("(未选择)"));
        for (auto& n : m_model->nodes) {
            const IInstruction* ii = InstructionRegistry::instance().get(n.instr);
            QString label = QString::fromStdString(n.id);
            if (ii) label += " · " + trInstr(ii->meta().name.c_str());
            cb->addItem(label, QString::fromStdString(n.id));
        }
        w = cb;
    } else { // string
        auto* le = new QLineEdit();
        le->setPlaceholderText(QString::fromStdString(p.def));
        w = le;
        if (p.key == "template") {
            // 找图/等待画面的模板图片：旁边加“截图选取”按钮，从屏幕裁剪模板
            container = new QWidget();
            auto* hl = new QHBoxLayout(container);
            hl->setContentsMargins(0, 0, 0, 0);
            hl->setSpacing(6);
            auto* btn = new QPushButton(tr("截图选取"));
            QString key = QString::fromStdString(p.key);
            connect(btn, &QPushButton::clicked, this,
                    [this, le, key] { pickTemplateFromScreen(le, key); });
            hl->addWidget(le, 1);
            hl->addWidget(btn);
        }
    }
    // 注意：m_widgets 始终保存 QLineEdit 本身，保证 readParam / writeBack 逻辑不变
    lay->addWidget(container ? container : w);
    m_widgets.insert(QString::fromStdString(p.key), w);

    // 行内错误提示（默认隐藏，就地校验时显示）
    auto* err = new QLabel();
    err->setObjectName("fieldError");
    err->setStyleSheet(QStringLiteral("color:%1;").arg(Palette::stop(ThemeManager::instance().effectiveDark()).name()));
    err->setWordWrap(true);
    err->setVisible(false);
    lay->addWidget(err);
    m_errors.insert(QString::fromStdString(p.key), err);

    if (!p.hint.empty()) {
        auto* hint = new QLabel(tr("示例: ") + trInstr(p.hint.c_str()));
        hint->setObjectName("fieldHint");
        hint->setWordWrap(true);
        lay->addWidget(hint);
    }
}

json ParamPanel::readParam(const QString& key) const {
    QWidget* w = m_widgets.value(key);
    if (!w) return json("");
    if (auto* le = qobject_cast<QLineEdit*>(w)) return le->text().toStdString();
    if (auto* st = qobject_cast<Stepper*>(w))
        return st->decimals() == 0 ? json((int)st->value()) : json(st->value());
    if (auto* cb = qobject_cast<QComboBox*>(w)) {
        if (cb->currentData().isValid()) return cb->currentData().toString().toStdString();
        return std::string();   // node 型"(未选择)"等无 data 项 → 空字符串，供汇总校验标记未选择
    }
    if (auto* ch = qobject_cast<QCheckBox*>(w)) return ch->isChecked();
    if (auto* te = qobject_cast<QPlainTextEdit*>(w)) return te->toPlainText().toStdString();
    return json("");
}

void ParamPanel::setParamWidget(QWidget* w, const std::string& type, const json& value,
                                const std::string& def) {
    std::string s;
    if (value.is_string()) s = value.get<std::string>();
    else if (value.is_number()) s = fmtNumber(value.get<double>());
    else if (value.is_boolean()) s = value.get<bool>() ? "true" : "false";
    else if (!value.is_null()) s = value.dump();
    else s = def;

    if (auto* le = qobject_cast<QLineEdit*>(w)) {
        le->setText(QString::fromStdString(s));
    } else if (auto* st = qobject_cast<Stepper*>(w)) {
        bool ok = false; double v = QString::fromStdString(s).toDouble(&ok);
        st->setValue(ok ? v : 0);
    } else if (auto* cb = qobject_cast<QComboBox*>(w)) {
        // node 与 select 都以 data 存原值（select 的显示文本可能是翻译后的中文）
        int idx = cb->findData(QString::fromStdString(s));
        cb->setCurrentIndex(idx < 0 ? 0 : idx);
    } else if (auto* ch = qobject_cast<QCheckBox*>(w)) {
        ch->setChecked(s == "true" || s == "1" || s == "是");
    } else if (auto* te = qobject_cast<QPlainTextEdit*>(w)) {
        te->setPlainText(QString::fromStdString(s));
    }
}

void ParamPanel::writeBack(const QString& key) {
    if (m_updating || m_nodeId.isEmpty()) return;
    FlowNode* n = m_model->nodeById(m_nodeId.toStdString());
    if (!n) return;
    n->params[key.toStdString()] = readParam(key);
    validateParam(key);
    emit paramEdited(m_nodeId);
}

bool ParamPanel::paramIsEmpty(const QString& key) const {
    QWidget* w = m_widgets.value(key);
    if (!w) return false;
    if (auto* le = qobject_cast<QLineEdit*>(w))
        return le->text().trimmed().isEmpty();
    if (auto* te = qobject_cast<QPlainTextEdit*>(w))
        return te->toPlainText().trimmed().isEmpty();
    if (auto* cb = qobject_cast<QComboBox*>(w)) {
        // node 型：未选择 = 空；select 型始终有值
        const QVariant d = cb->currentData();
        return !d.isValid() || d.toString().isEmpty();
    }
    return false;
}

void ParamPanel::validateParam(const QString& key) {
    QWidget* w = m_widgets.value(key);
    auto it = m_paramMeta.constFind(key);
    if (!w || it == m_paramMeta.constEnd()) return;

    // 数值控件：越界由 Stepper 自行标红，这里只联动行内提示
    if (qobject_cast<Stepper*>(w)) {
        const bool invalid = static_cast<Stepper*>(w)->hasRangeError();
        setParamError(key, invalid, invalid ? tr("数值超出范围") : QString());
        return;
    }

    const bool invalid = it->required && paramIsEmpty(key);
    setParamError(key, invalid, invalid ? tr("此参数不能为空") : QString());
}

bool ParamPanel::validateCurrent() const {
    if (!m_instr) return true;
    bool allValid = true;
    for (auto it = m_paramMeta.constBegin(); it != m_paramMeta.constEnd(); ++it) {
        const QString key = it.key();
        QWidget* w = m_widgets.value(key);
        if (!w) continue;
        if (auto* st = qobject_cast<Stepper*>(w)) {
            if (st->hasRangeError()) allValid = false;
        } else if (it->required && paramIsEmpty(key)) {
            allValid = false;
        }
    }
    return allValid;
}

void ParamPanel::setParamError(const QString& key, bool invalid, const QString& msg) {
    if (QLabel* err = m_errors.value(key)) {
        err->setVisible(invalid);
        if (invalid) err->setText(msg);
    }
    QWidget* w = m_widgets.value(key);
    if (!w || qobject_cast<Stepper*>(w)) return;   // Stepper 自行管理红色边框
    w->setStyleSheet(invalid ? QStringLiteral("border: 1px solid %1;").arg(Palette::stop(ThemeManager::instance().effectiveDark()).name()) : QString());
    w->style()->unpolish(w);
    w->style()->polish(w);
}

QString ParamPanel::labelText(const QString& base, bool required) const {
    if (required)
        return base + QStringLiteral(" <span style='color:%1'>*</span>").arg(Palette::stop(ThemeManager::instance().effectiveDark()).name());
    return base + QStringLiteral(" <span style='color:%1'>").arg(Palette::textMute(ThemeManager::instance().effectiveDark()).name()) + tr("(可选)") +
           QStringLiteral("</span>");
}

void ParamPanel::pickTemplateFromScreen(QLineEdit* le, const QString& key) {
    // 1) 隐藏主窗口，避免把自己截进画面
    QWidget* top = window();
    if (top) top->hide();
    qApp->processEvents();
    QThread::msleep(250); // 等窗口隐藏与桌面重绘完成

    // 2) 截取全屏（沿用找图指令的临时目录命名规则：%TEMP%/autoflow/shot_*.png）
    QString dir = QDir::tempPath() + "/autoflow";
    QDir().mkpath(dir);
    QString shotPath = dir + "/shot_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    if (!ScreenCapture::captureToPng(shotPath.toStdString())) {
        if (top) { top->show(); top->raise(); top->activateWindow(); }
        return;
    }
    QPixmap bg(shotPath);

    // 3) 弹出全屏裁剪遮罩，用局部事件循环等待框选结果
    auto* overlay = new CropOverlay(bg);
    QEventLoop loop;
    QRect selected;
    bool confirmed = false;
    connect(overlay, &CropOverlay::regionSelected, this,
            [&](const QRect& r) { selected = r; confirmed = true; loop.quit(); });
    connect(overlay, &CropOverlay::cancelled, &loop, &QEventLoop::quit);
    overlay->showFullScreen();
    overlay->raise();
    overlay->activateWindow();
    overlay->setFocus();
    loop.exec();
    overlay->close(); // WA_DeleteOnClose 自动销毁

    // 4) 恢复主窗口
    if (top) { top->show(); top->raise(); top->activateWindow(); }
    if (!confirmed || selected.isEmpty()) return;

    // 5) 裁剪选区，另存为新 PNG 并回填路径（setText 不触发 textEdited，需手动写回）
    QImage full(shotPath);
    QImage cropped = full.copy(selected);
    if (cropped.isNull()) return;
    QString cropPath = dir + "/crop_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    if (!cropped.save(cropPath, "PNG")) return;
    le->setText(cropPath);
    writeBack(key);
}

} // namespace autoflow
