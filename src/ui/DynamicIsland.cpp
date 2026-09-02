#include "DynamicIsland.h"
#include "Palette.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QFontMetrics>
#include <algorithm>

namespace autoflow {

static const bool kDark = true;

DynamicIsland::DynamicIsland(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    m_anim = new QPropertyAnimation(this, "expansion", this);
    m_anim->setDuration(300);
    m_anim->setEasingCurve(QEasingCurve::InOutCubic);

    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(50);
    connect(m_tickTimer, &QTimer::timeout, this, &DynamicIsland::onTick);

    setIdle();
    show();
}

void DynamicIsland::onTick() {
    update();
}

void DynamicIsland::setExpansion(qreal v) {
    m_expansion = v;
    updateSize();
    update();
}

void DynamicIsland::updateSize() {
    int w = 120 + (int)(220 * m_expansion);
    if (m_expansion > 0.3) {
        QFont f = font();
        f.setPixelSize(12);
        QFontMetrics fm(f);
        int contentW = 0;
        contentW += fm.horizontalAdvance(m_moduleName) + 20;
        for (const auto& p : m_params) {
            int lineW = fm.horizontalAdvance(p.label + ": " + p.value) + 20;
            contentW = std::max(contentW, lineW);
        }
        w = std::max(w, std::min(contentW, 500));
    }
    setFixedSize(w, 36);
    reposition();
}

void DynamicIsland::reposition() {
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        const QRect sg = screen->availableGeometry();
        const int x = (sg.width() - width()) / 2 + sg.x();
        move(x, sg.y() + 8);
    }
}

void DynamicIsland::animateTo(qreal target) {
    m_anim->stop();
    m_anim->setStartValue(m_expansion);
    m_anim->setEndValue(target);
    m_anim->start();
}

void DynamicIsland::setIdle() {
    m_moduleName = QString::fromUtf8("空闲");
    m_params.clear();
    m_dotColor = Palette::textMute(kDark);
    m_progressTotal = 0;
    m_tickTimer->stop();
    animateTo(0.0);
}

void DynamicIsland::setRunning(const QString& moduleName, const QList<IslandParam>& params,
                                int progressMs) {
    m_moduleName = moduleName;
    m_params = params;
    m_dotColor = Palette::accent(kDark);
    m_progressTotal = progressMs;
    m_stepElapsed.start();
    m_tickTimer->start();
    animateTo(1.0);
}

void DynamicIsland::setResult(const QString& summary) {
    m_moduleName = summary;
    m_params.clear();
    m_dotColor = Palette::run(kDark);
    m_progressTotal = 0;
    m_tickTimer->stop();
    animateTo(1.0);
    QTimer::singleShot(3000, this, [this] { setIdle(); });
}

void DynamicIsland::setError(const QString& msg) {
    m_moduleName = msg;
    m_params.clear();
    m_dotColor = Palette::stop(kDark);
    m_progressTotal = 0;
    m_tickTimer->stop();
    animateTo(1.0);
    QTimer::singleShot(4000, this, [this] { setIdle(); });
}

void DynamicIsland::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 纯黑胶囊底
    const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0));
    p.drawRoundedRect(r, 18, 18);

    // 左侧状态色点
    const qreal dotR = 4.0;
    const qreal dotX = 16.0;
    const qreal dotY = r.center().y();
    p.setBrush(m_dotColor);
    p.drawEllipse(QPointF(dotX, dotY), dotR, dotR);

    // 文字
    QFont f = font();
    f.setPixelSize(12);
    f.setWeight(QFont::Medium);
    p.setFont(f);
    p.setPen(QColor(255, 255, 255));

    const qreal textX = dotX + 12;
    const qreal textY = 0;

    // 右侧时间文字（运行中显示已运行时长）
    QString timeText;
    bool running = m_progressTotal > 0 || m_tickTimer->isActive();
    if (running && m_stepElapsed.isValid()) {
        qint64 ms = m_stepElapsed.elapsed();
        if (ms < 1000)
            timeText = QString::number(ms) + "ms";
        else
            timeText = QString::number(ms / 1000.0, 'f', 1) + "s";
    }

    if (m_expansion < 0.4) {
        QString shortText = m_moduleName.length() > 2 ? m_moduleName.left(2) : m_moduleName;
        const QRectF textRect(textX, textY, r.width() - textX - 8, r.height());
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, shortText);
    } else {
        QFontMetrics fm(f);

        if (!timeText.isEmpty()) {
            QFont tf = f;
            tf.setPixelSize(11);
            tf.setWeight(QFont::Normal);
            p.setFont(tf);
            p.setPen(QColor(160, 160, 160));
            int timeW = QFontMetrics(tf).horizontalAdvance(timeText);
            QRectF timeRect(r.width() - timeW - 12, textY, timeW + 4, r.height());
            p.drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timeText);
            p.setFont(f);
            p.setPen(QColor(255, 255, 255));
        }

        QRectF textRect(textX, textY, r.width() - textX - (timeText.isEmpty() ? 8 : 60), r.height());
        if (m_params.isEmpty()) {
            p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_moduleName);
        } else {
            QString line = m_moduleName;
            for (const auto& param : m_params)
                line += "  ·  " + param.label + ": " + param.value;
            int availW = (int)textRect.width();
            if (fm.horizontalAdvance(line) > availW) {
                p.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, m_moduleName);
                QFont f2 = f;
                f2.setPixelSize(11);
                f2.setWeight(QFont::Normal);
                p.setFont(f2);
                p.setPen(QColor(200, 200, 200));
                QString paramLine;
                for (const auto& param : m_params)
                    paramLine += param.label + ": " + param.value + "  ";
                paramLine = paramLine.trimmed();
                QRectF paramRect(textX, textY + 16, r.width() - textX - 8, r.height() - 16);
                if (fm.horizontalAdvance(paramLine) > (int)paramRect.width())
                    paramLine = fm.elidedText(paramLine, Qt::ElideRight, (int)paramRect.width());
                p.drawText(paramRect, Qt::AlignLeft | Qt::AlignTop, paramLine);
            } else {
                p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, line);
            }
        }
    }

    // 确定性进度条：仅在时间类模块（progressTotal > 0）运行时显示
    // 按实际时长从 0% → 100% 填充，非时间模块不显示
    if (m_progressTotal > 0 && m_expansion > 0.3 && m_stepElapsed.isValid()) {
        const qreal barY = r.bottom() - 3;
        const qreal barH = 2.0;
        const qreal barLeft = 8;
        const qreal barRight = r.right() - 8;
        const qreal barW = barRight - barLeft;

        // 底部轨道
        p.setPen(Qt::NoPen);
        QColor trackColor = m_dotColor;
        trackColor.setAlphaF(0.12);
        p.setBrush(trackColor);
        p.drawRoundedRect(QRectF(barLeft, barY, barW, barH), 1, 1);

        // 确定性填充：elapsed / total（夹到 0~1）
        qreal progress = (qreal)m_stepElapsed.elapsed() / (qreal)m_progressTotal;
        progress = std::clamp(progress, 0.0, 1.0);

        QColor barColor = m_dotColor;
        p.setBrush(barColor);
        p.drawRoundedRect(QRectF(barLeft, barY, barW * progress, barH), 1, 1);
    }
}

} // namespace autoflow
