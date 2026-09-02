#include "ThemeToggle.h"
#include "ThemeManager.h"
#include "Palette.h"
#include "IconPainter.h"

#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QtMath>

namespace autoflow {

// 边界阻力（apple-design rubber-band）
static qreal rubberBand(qreal overshoot, qreal dim, qreal c = 0.55) {
    return (overshoot * dim * c) / (dim + c * qAbs(overshoot));
}

ThemeToggle::ThemeToggle(QWidget* parent) : QAbstractButton(parent) {
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_Hover);
    setAccessibleName(tr("切换深色/浅色主题"));

    m_anim = new QPropertyAnimation(this, "progress", this);
    m_anim->setDuration(260);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(this, &QAbstractButton::toggled, this, [this](bool dark) {
        animateProgress(dark ? 1.0 : 0.0);
    });
}

void ThemeToggle::setDark(bool dark) { setChecked(dark); }

void ThemeToggle::animateProgress(qreal target) {
    m_anim->stop();
    m_anim->setStartValue(m_progress);
    m_anim->setEndValue(target);
    m_anim->start();
}

// ---- 几何 ----
// 旋钮直径 28，左缘滑动范围 x=3..33 → 圆心 x=17..47
qreal ThemeToggle::knobMinX() const { return 3 + 14; }
qreal ThemeToggle::knobMaxX() const { return 33 + 14; }
qreal ThemeToggle::knobCenterX() const { return knobMinX() + m_progress * (knobMaxX() - knobMinX()); }

// ---- 拖动 ----
void ThemeToggle::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragMoved = false;
        m_dragStartMouseX = e->pos().x();
        m_dragStartCenterX = knobCenterX();
        m_anim->stop();
    }
    QAbstractButton::mousePressEvent(e);
}

void ThemeToggle::mouseMoveEvent(QMouseEvent* e) {
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        qreal dx = e->pos().x() - m_dragStartMouseX;
        if (qAbs(dx) > 3) m_dragMoved = true;
        if (m_dragMoved) {
            qreal targetX = m_dragStartCenterX + dx;
            qreal mn = knobMinX(), mx = knobMaxX();
            qreal range = mx - mn;
            if (targetX < mn) targetX = mn - rubberBand(mn - targetX, range);
            else if (targetX > mx) targetX = mx + rubberBand(targetX - mx, range);
            setProgress((targetX - mn) / range);
        }
    }
    QAbstractButton::mouseMoveEvent(e);
}

void ThemeToggle::mouseReleaseEvent(QMouseEvent* e) {
    if (m_dragging) {
        m_dragging = false;
        if (!m_dragMoved) {
            // 点击：切换
            setChecked(!isChecked());
        } else {
            // 拖动：吸附到近端
            bool dark = (m_progress >= 0.5);
            setChecked(dark);
            animateProgress(dark ? 1.0 : 0.0);   // 确保吸附（即使 toggled 未触发）
        }
        return;
    }
    QAbstractButton::mouseReleaseEvent(e);
}

// ---- 绘制 ----
void ThemeToggle::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const bool dark = ThemeManager::instance().effectiveDark();

    const QRectF track = QRectF(rect());   // 64×34，圆角 16

    // 轨道：纯色（开=主题色，关=surface2）
    p.setPen(Qt::NoPen);
    p.setBrush(m_progress >= 0.5 ? Palette::accent(dark) : Palette::surface2(dark));
    p.drawRoundedRect(track, 16, 16);

    // 旋钮：纯色（浅色白 / 深色 surface2）
    const QPointF knobC(knobCenterX(), 17);
    p.setPen(Qt::NoPen);
    p.setBrush(dark ? Palette::surface2(dark) : Palette::bg(dark));
    p.drawEllipse(knobC, 13, 13);

    // 图标交叉淡化：太阳(关) / 月亮(开)，统一线性风格
    const QRectF iconBox(knobC.x() - 9, knobC.y() - 9, 18, 18);
    if (m_progress < 1.0) { p.save(); p.setOpacity(1.0 - m_progress);
        IconPainter::paint(p, iconBox, IconPainter::Id::Sun, Palette::warn(dark), 1.75); p.restore(); }
    if (m_progress > 0.0) { p.save(); p.setOpacity(m_progress);
        IconPainter::paint(p, iconBox, IconPainter::Id::Moon, Palette::text(dark), 1.75); p.restore(); }
}

} // namespace autoflow
