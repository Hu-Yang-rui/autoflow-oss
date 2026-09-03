#include "RunBorder.h"
#include "Palette.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QHideEvent>
#include <QLinearGradient>
#include <cmath>
#include <algorithm>

namespace autoflow {

RunBorder::RunBorder(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);   // 鼠标穿透，不挡住任何操作
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 动画定时器：约 30fps 驱动呼吸脉动
    m_timer = new QTimer(this);
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, [this] {
        m_phase += 0.10;
        update();
    });
}

void RunBorder::showOnScreen() {
    if (QScreen* s = QApplication::primaryScreen()) {
        setGeometry(s->geometry());
    }
    m_phase = 0.0;
    show();
    raise();
    m_timer->start();
}

void RunBorder::hideEvent(QHideEvent*) {
    m_timer->stop();
}

void RunBorder::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const bool dark = ThemeManager::instance().isDark();
    const QColor base = Palette::accent(dark);

    // 呼吸脉动：0.4 ~ 1.0
    const qreal pulse = 0.4 + 0.6 * (0.5 + 0.5 * std::sin(m_phase));

    const int W = width(), H = height();
    // 光带向内扩散宽度：与屏幕短边成比例，任何分辨率下比例都协调
    const int band = std::max(30, (int)std::lround(std::min(W, H) * 0.05));

    QColor c0 = base; c0.setAlpha((int)std::lround(165 * pulse));   // 边缘最亮
    QColor c1 = base; c1.setAlpha((int)std::lround(40 * pulse));    // 中间衰减
    QColor c2 = base; c2.setAlpha(0);                               // 末端透明

    // 四边柔和渐变光带（从屏幕边缘向内平滑晕开，无硬边）
    // 上边
    {
        QLinearGradient g(0, 0, 0, band);
        g.setColorAt(0.0, c0); g.setColorAt(0.55, c1); g.setColorAt(1.0, c2);
        p.fillRect(QRect(0, 0, W, band), g);
    }
    // 下边
    {
        QLinearGradient g(0, H, 0, H - band);
        g.setColorAt(0.0, c0); g.setColorAt(0.55, c1); g.setColorAt(1.0, c2);
        p.fillRect(QRect(0, H - band, W, band), g);
    }
    // 左边
    {
        QLinearGradient g(0, 0, band, 0);
        g.setColorAt(0.0, c0); g.setColorAt(0.55, c1); g.setColorAt(1.0, c2);
        p.fillRect(QRect(0, 0, band, H), g);
    }
    // 右边
    {
        QLinearGradient g(W, 0, W - band, 0);
        g.setColorAt(0.0, c0); g.setColorAt(0.55, c1); g.setColorAt(1.0, c2);
        p.fillRect(QRect(W - band, 0, band, H), g);
    }
}

} // namespace autoflow
