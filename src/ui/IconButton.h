#pragma once
#include <QAbstractButton>

class QPropertyAnimation;

namespace autoflow {

// 统一的矢量图标按钮（+ / − / chevron / 窗口控制）
// 悬停/按下状态即时刷新：WA_Hover + 立即 update() + unpolish/polish，150ms 过渡
class IconButton : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal hover READ hover WRITE setHover)
    Q_PROPERTY(qreal press READ press WRITE setPress)
public:
    enum Icon { Plus, Minus, Chevron, WinMinimize, WinMaximize, WinRestore, WinClose };

    explicit IconButton(Icon icon = Plus, QWidget* parent = nullptr);

    void setIconType(Icon icon) { m_icon = icon; update(); updateAccessibleName(); }
    Icon iconType() const { return m_icon; }

    // close 按钮专用：悬停/按下时用危险色（红）实心背景 + 反色图标
    void setDanger(bool on) { m_danger = on; update(); }
    bool danger() const { return m_danger; }

    qreal hover() const { return m_hover; }
    void setHover(qreal v) { m_hover = v; update(); }
    qreal press() const { return m_press; }
    void setPress(qreal v) { m_press = v; update(); }

    QSize sizeHint() const override { return QSize(22, 22); }

protected:
    void paintEvent(QPaintEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    void runAnim(QPropertyAnimation* anim, qreal from, qreal to);
    void refreshStyle();   // 状态切换后强制刷新样式 + 重绘
    void updateAccessibleName();   // 按图标类型设置无障碍标签

    Icon m_icon = Plus;
    bool m_danger = false;
    qreal m_hover = 0.0;   // 0..1 悬停过渡
    qreal m_press = 0.0;   // 0..1 按下过渡
    QPropertyAnimation* m_hoverAnim = nullptr;
    QPropertyAnimation* m_pressAnim = nullptr;
};

} // namespace autoflow
