#include "RunBorder.h"
#include "Palette.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QHideEvent>
#include <cmath>

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
        // 读取屏幕参数：按物理 DPI 计算边框粗细比例（高 DPI 下加粗，避免比例失调）
        const qreal dpi = s->physicalDotsPerInch();
        m_scale = qBound(1.0, dpi / 96.0, 2.0);
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

    // 呼吸脉动：0.25 ~ 1.0，让发光有节奏地起伏
    const qreal pulse = 0.3 + 0.7 * (0.5 + 0.5 * std::sin(m_phase));

    const int m = (int)std::lround(18.0 * m_scale);      // 边框距屏幕边缘
    QRect r = rect().adjusted(m, m, -m, -m);

    // 光晕：向屏幕内侧多层扩散，透明度随呼吸脉动（由浓到淡）
    const int N = 24;
    for (int i = 0; i < N; ++i) {
        int alpha = (int)std::lround((88 - i * 3.6) * pulse);
        alpha = qBound(0, alpha, 255);
        QColor c = base;
        c.setAlpha(alpha);
        int w = (int)std::lround(2.0 * m_scale);
        p.setPen(QPen(c, w));
        int off = (int)std::lround(i * 1.2 * m_scale);
        p.drawRect(r.adjusted(off, off, -off, -off));
    }

    // 主体高亮边框（最亮，随呼吸微微变化）
    QColor hi = base.lighter(165);
    hi.setAlpha((int)std::lround(235 * (0.75 + 0.25 * pulse)));
    p.setPen(QPen(hi, (int)std::lround(3.0 * m_scale)));
    p.drawRect(r);
}

} // namespace autoflow
