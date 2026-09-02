#include "LogPanel.h"
#include "ThemeManager.h"
#include "Palette.h"
#include "../core/Settings.h"

#include <QHeaderView>
#include <QTime>
#include <QColor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <algorithm>

namespace autoflow {

namespace {

// 把 QColor 转成 QSS 可用字符串（半透明用 rgba()，不透明用 #RRGGBB）
QString cssColor(const QColor& c) {
    if (c.alpha() == 255)
        return c.name();
    return QStringLiteral("rgba(%1,%2,%3,%4)")
        .arg(c.red()).arg(c.green()).arg(c.blue())
        .arg(c.alphaF(), 0, 'f', 3);
}

// 日志级别 → 前景色（info 走 muted 次级色）
QColor levelColor(const QString& level, bool dark) {
    if (level == QLatin1String("success")) return Palette::run(dark);
    if (level == QLatin1String("warn"))    return Palette::warn(dark);
    if (level == QLatin1String("error"))   return Palette::stop(dark);
    return Palette::textDim(dark);
}

// 圆角行 + 状态胶囊 delegate：hover 画圆角背景，状态列画圆角胶囊
class LogDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        const bool dark = ThemeManager::instance().effectiveDark();

        // hover：圆角行背景
        if (option.state & QStyle::State_MouseOver) {
            QRectF r = QRectF(option.rect).adjusted(3, 1, -3, -1);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Palette::surface2(dark));
            painter->drawRoundedRect(r, 8, 8);
            painter->restore();
        }

        // 状态列（column 1）：圆角胶囊
        if (index.column() == 1) {
            const QString level = index.data(Qt::UserRole).toString();
            const QString text = index.data().toString();
            const QColor fg = levelColor(level, dark);
            QColor bg = fg;
            bg.setAlphaF(0.15f);
            QFont f = option.font;
            f.setPixelSize(11);
            const QFontMetrics fm(f);
            const qreal pillW = fm.horizontalAdvance(text) + 18.0;
            const qreal pillH = 20.0;
            const QRectF pill(option.rect.left() + 4.0,
                              option.rect.center().y() - pillH / 2.0,
                              pillW, pillH);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(Qt::NoPen);
            painter->setBrush(bg);
            painter->drawRoundedRect(pill, pillH / 2.0, pillH / 2.0);
            painter->setFont(f);
            painter->setPen(fg);
            painter->drawText(pill, Qt::AlignCenter, text);
            painter->restore();
            return;
        }

        // 其他列：默认绘制（去焦点框）
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.state &= ~QStyle::State_HasFocus;
        QStyledItemDelegate::paint(painter, opt, index);
    }
};

} // namespace

LogPanel::LogPanel(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    // 标题行：运行日志 + 清空/复制（ghost 按钮）
    auto* head = new QHBoxLayout();
    head->setContentsMargins(8, 6, 8, 6);

    m_title = new QLabel(tr("运行日志"), this);
    head->addWidget(m_title);
    head->addStretch(1);

    m_clearBtn = new QPushButton(tr("清空"), this);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &LogPanel::clearLog);

    m_copyBtn = new QPushButton(tr("复制"), this);
    m_copyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_copyBtn, &QPushButton::clicked, this, &LogPanel::copySelectedLog);

    head->addWidget(m_clearBtn);
    head->addWidget(m_copyBtn);
    lay->addLayout(head);

    m_table = new QTableWidget(this);
    m_table->setObjectName("logTable");
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({ tr("时间"), tr("状态"), tr("步骤"), tr("消息"), tr("耗时") });
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setVisible(false);   // 圆角风格：无表头
    m_table->setItemDelegate(new LogDelegate(m_table));   // 圆角行 + 状态胶囊
    m_table->setShowGrid(false);                     // 无网格线：行用底边框分隔
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(false);
    m_table->setWordWrap(true);
    m_table->setFont(ThemeManager::smileySansFont());
    m_table->horizontalHeader()->setFont(ThemeManager::smileySansFont());
    lay->addWidget(m_table);

    applyStyle();

    // 深/浅主题切换时刷新内联样式与已渲染行的级别着色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](bool) { applyStyle(); });
}

void LogPanel::appendLog(const QString& level, const QString& nodeId, const QString& nodeName,
                         const QString& text, qint64 elapsedMs) {
    int row = m_table->rowCount();
    m_table->insertRow(row);

    const bool dark = ThemeManager::instance().effectiveDark();
    QColor color = levelColor(level, dark);
    QString levelText = tr("信息");
    if (level == "success") levelText = tr("成功");
    else if (level == "warn") levelText = tr("警告");
    else if (level == "error") levelText = tr("失败");

    auto* timeItem = new QTableWidgetItem(QTime::currentTime().toString("HH:mm:ss.zzz"));
    auto* levelItem = new QTableWidgetItem(levelText);
    levelItem->setData(Qt::UserRole, level);   // 存原始级别（info/success/warn/error），供过滤
    auto* nodeItem = new QTableWidgetItem(nodeName.isEmpty() ? nodeId : nodeName);
    auto* msgItem = new QTableWidgetItem(text);
    auto* msItem = new QTableWidgetItem(elapsedMs > 0 ? QString::number(elapsedMs) + "ms" : "");

    levelItem->setForeground(color);
    msItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // 时间戳 / 耗时用等宽字体（仪器读数感）
    QFont mono(QStringLiteral("Consolas"));
    mono.setPixelSize(11);
    timeItem->setFont(mono);
    msItem->setFont(mono);

    m_table->setItem(row, 0, timeItem);
    m_table->setItem(row, 1, levelItem);
    m_table->setItem(row, 2, nodeItem);
    m_table->setItem(row, 3, msgItem);
    m_table->setItem(row, 4, msItem);
    applyLevelFilter();
    m_table->scrollToBottom();

    // 行数上限：0 = 不限；超出时从顶部移除最旧的行
    const int cap = Settings::instance().logMaxLines();
    if (cap > 0) {
        while (m_table->rowCount() > cap)
            m_table->removeRow(0);
    }

    // 可选落盘日志
    if (Settings::instance().logToFile())
        appendLogToFile(level, nodeName.isEmpty() ? nodeId : nodeName, text);
}

void LogPanel::appendLogToFile(const QString& level, const QString& nodeName, const QString& text) {
    if (!m_logFile.isOpen()) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dir.isEmpty())
            dir = QDir::tempPath();
        QDir().mkpath(dir);
        m_logFile.setFileName(dir + "/autofeed.log");
        if (!m_logFile.open(QIODevice::Append | QIODevice::Text))
            return;   // 打不开就静默跳过本次落盘
    }
    QTextStream ts(&m_logFile);
    ts << '[' << QTime::currentTime().toString("HH:mm:ss.zzz") << "] "
       << level << ' ' << nodeName << ' ' << text << '\n';
    ts.flush();
}

void LogPanel::clearLog() { m_table->setRowCount(0); }

void LogPanel::applyStyle() {
    const bool dark = ThemeManager::instance().effectiveDark();

    // 标题：主文字、中等字重
    m_title->setStyleSheet(QStringLiteral("font-size:12px; font-weight:600; color:%1;")
                           .arg(Palette::text(dark).name()));

    // ghost 按钮：透明底、无边框，hover 出现次级底色
    const QString ghost = QStringLiteral(
        "QPushButton { background: transparent; border: none; border-radius: 6px;"
        " padding: 4px 10px; color: %1; font-size: 12px; }"
        "QPushButton:hover { background: %2; color: %3; }"
        "QPushButton:pressed { background: %4; }"
        "QPushButton:focus { outline: 2px solid %5; }")
        .arg(cssColor(Palette::textSecondary(dark)),
             cssColor(Palette::surface2(dark)),
             cssColor(Palette::text(dark)),
             cssColor(Palette::surface3(dark)),
             cssColor(Palette::accentSubtle(dark)));
    m_clearBtn->setStyleSheet(ghost);
    m_copyBtn->setStyleSheet(ghost);

    // 表格：无网格线、无分隔线（圆角行风格，hover/胶囊由 delegate 绘制）
    const QString table = QStringLiteral(
        "QTableWidget#logTable { background: %1; border: none; font-size: 12px; }"
        "QTableWidget#logTable::item { border: none; padding: 7px 10px; }")
        .arg(cssColor(Palette::surface(dark)));
    m_table->setStyleSheet(table);

    // 表头：muted-foreground，底部 1px 分隔
    m_table->horizontalHeader()->setStyleSheet(QStringLiteral(
        "QHeaderView::section { background: transparent; border: none;"
        " border-bottom: 1px solid %1; color: %2; padding: 6px 10px; font-weight: 500; font-size: 12px; }")
        .arg(cssColor(Palette::borderStrong(dark)),
             cssColor(Palette::textSecondary(dark))));

    refreshLevelColors();
}

void LogPanel::refreshLevelColors() {
    const bool dark = ThemeManager::instance().effectiveDark();
    for (int i = 0; i < m_table->rowCount(); ++i) {
        QTableWidgetItem* it = m_table->item(i, 1);
        if (it)
            it->setForeground(levelColor(it->data(Qt::UserRole).toString(), dark));
    }
}

void LogPanel::applyLevelFilter() {
    const QString filter = m_levelFilter ? m_levelFilter->currentData().toString()
                                         : QStringLiteral("all");
    for (int i = 0; i < m_table->rowCount(); ++i) {
        QTableWidgetItem* it = m_table->item(i, 1);
        const QString level = it ? it->data(Qt::UserRole).toString() : QString();
        const bool show = (filter == QStringLiteral("all") || filter == level);
        m_table->setRowHidden(i, !show);
    }
}

void LogPanel::copySelectedLog() {
    QList<int> rows;
    const auto sel = m_table->selectionModel()->selectedRows();
    for (const QModelIndex& idx : sel)
        rows << idx.row();
    if (rows.isEmpty()) {
        for (int i = 0; i < m_table->rowCount(); ++i)
            if (!m_table->isRowHidden(i))
                rows << i;
    }
    std::sort(rows.begin(), rows.end());

    QStringList lines;
    for (int r : rows) {
        QStringList cols;
        for (int c = 0; c < m_table->columnCount(); ++c) {
            QTableWidgetItem* it = m_table->item(r, c);
            cols << (it ? it->text() : QString());
        }
        lines << cols.join(QLatin1Char('\t'));
    }
    if (!lines.isEmpty())
        QApplication::clipboard()->setText(lines.join(QLatin1Char('\n')));
}

void LogPanel::setLastTiming(qint64 ms) {
    int row = m_table->rowCount() - 1;
    if (row < 0) return;
    QTableWidgetItem* item = m_table->item(row, 4);
    if (!item) {
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, 4, item);
    }
    item->setText(ms > 0 ? QString::number(ms) + "ms" : "");
}

} // namespace autoflow
