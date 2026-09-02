#include "IconButton.h"
#include "ThemeManager.h"
#include "Palette.h"
#include "IconPainter.h"

#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QStyle>

namespace autoflow {

static QColor mixColor(const QColor& a, const QColor& b, qreal t) {
    return QColor(
        qRound(a.red()   + (b.red()   - a.red())   * t),
        qRound(a.green() + (b.green() - a.green()) * t),
        qRound(a.blue()  + (b.blue()  - a.blue())  * t),
        qRound(a.alpha() + (b.alpha() - a.alpha()) * t)
    );
}

static IconPainter::Id toPainterId(IconButton::Icon ic) {
    switch (ic) {
    case IconButton::Plus:         return IconPainter::Id::Plus;
    case IconButton::Minus:        return IconPainter::Id::Minus;
    case IconButton::Chevron:      return IconPainter::Id::ChevronDown;
    case IconButton::WinMinimize:  return IconPainter::Id::WinMinimize;
    case IconButton::WinMaximize:  return IconPainter::Id::WinMaximize;
    case IconButton::WinRestore:   return IconPainter::Id::WinRestore;
    case IconButton::WinClose:     return IconPainter::Id::WinClose;
    }
    return IconPainter::Id::Plus;
}

IconButton::IconButton(Icon icon, QWidget* parent)
    : QAbstractButton(parent), m_icon(icon) {
    setFixedSize(22, 22);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_Hover);   // 确保悬停事件即时投递
    updateAccessibleName();

    m_hoverAnim = new QPropertyAnimation(this, "hover", this);
    m_hoverAnim->setDuration(150);
    m_hoverAnim->setEasingCurve(QEasingCurve::InOutQuad);   // 悬停：ease（平滑、不抢眼）

    m_pressAnim = new QPropertyAnimation(this, "press", this);
    m_pressAnim->setDuration(100);                           // 按下：ease-out，100ms 更跟手
    m_pressAnim->setEasingCurve(QEasingCurve::OutCubic);
}

void IconButton::runAnim(QPropertyAnimation* anim, qreal from, qreal to) {
    anim->stop();
    anim->setStartValue(from);
    anim->setEndValue(to);
    anim->start();
}

void IconButton::refreshStyle() {
    // 状态切换后强制刷新样式并立即重绘，避免“要点击别处才变化”
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void IconButton::updateAccessibleName() {
    switch (m_icon) {
    case Plus:         setAccessibleName(tr("增加")); break;
    case Minus:        setAccessibleName(tr("减少")); break;
    case Chevron:      setAccessibleName(tr("折叠/展开")); break;
    case WinMinimize:  setAccessibleName(tr("最小化")); break;
    case WinMaximize:  setAccessibleName(tr("最大化")); break;
    case WinRestore:   setAccessibleName(tr("还原")); break;
    case WinClose:     setAccessibleName(tr("关闭")); break;
    }
}

void IconButton::enterEvent(QEnterEvent* e) {
    runAnim(m_hoverAnim, m_hover, 1.0);   // 150ms 过渡，立即启动
    refreshStyle();                       // 立即刷新（不等待动画首帧）
    QAbstractButton::enterEvent(e);
}

void IconButton::leaveEvent(QEvent* e) {
    runAnim(m_hoverAnim, m_hover, 0.0);
    refreshStyle();
    QAbstractButton::leaveEvent(e);
}

void IconButton::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        runAnim(m_pressAnim, m_press, 1.0);
        refreshStyle();
    }
    QAbstractButton::mousePressEvent(e);
}

void IconButton::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        runAnim(m_pressAnim, m_press, 0.0);
        refreshStyle();
    }
    QAbstractButton::mouseReleaseEvent(e);
}

void IconButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 禁用态：整体 40% 透明度
    qreal globalAlpha = isEnabled() ? 1.0 : 0.4;

    // 按下缩放 0.9（以中心为基准）
    qreal scale = 1.0 - 0.1 * m_press;
    QRectF base = QRectF(rect()).adjusted(1, 1, -1, -1);
    QRectF box = base;
    if (scale < 1.0) {
        qreal w = base.width() * scale, h = base.height() * scale;
        box = QRectF(base.center().x() - w / 2, base.center().y() - h / 2, w, h);
    }

    bool dark = ThemeManager::instance().effectiveDark();
    const QColor theme = Palette::accent(dark);
    const QColor gray = Palette::textDim(dark);

    QColor iconColor;
    if (m_danger && m_hover > 0.0) {
        // close 按钮：危险色实心背景 + 反色图标
        QColor bg = Palette::stop(dark);
        bg.setAlphaF((0.85 + 0.15 * m_press) * m_hover * globalAlpha);
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(box, 4, 4);
        iconColor = Palette::bg(dark);
    } else {
        // 普通：悬停 10% 主题色背景，图标浅灰 -> 主题蓝
        if (m_hover > 0.0) {
            QColor bg = theme;
            bg.setAlphaF((0.10 + 0.10 * m_press) * m_hover * globalAlpha);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(box, 4, 4);
        }
        iconColor = mixColor(gray, theme, m_hover);
    }
    iconColor.setAlphaF(iconColor.alphaF() * globalAlpha);

    // 统一走 IconPainter，保证全 UI 图标同风格
    IconPainter::paint(p, box, toPainterId(m_icon), iconColor, 1.75);
}

} // namespace autoflow
