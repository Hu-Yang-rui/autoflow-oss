#pragma once
#include <QAbstractButton>
#include <QPointF>

class QPropertyAnimation;

namespace autoflow {

// 昼夜滑动开关：扁平轨道 + 线性太阳/月亮图标（统一 IconPainter 风格）；支持拖动 + 边界阻力(rubber-band)
class ThemeToggle : public QAbstractButton {
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)
public:
    explicit ThemeToggle(QWidget* parent = nullptr);

    qreal progress() const { return m_progress; }
    void setProgress(qreal v) { m_progress = v; update(); }

    void setDark(bool dark);
    bool isDark() const { return m_progress >= 0.5; }

    QSize sizeHint() const override { return QSize(64, 34); }

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    void animateProgress(qreal target);

    // 拖动几何（64×34，旋钮直径 28，滑动范围 x=3..33）
    qreal knobMinX() const;
    qreal knobMaxX() const;
    qreal knobCenterX() const;

    qreal m_progress = 0.0;
    QPropertyAnimation* m_anim = nullptr;

    bool m_dragging = false;
    bool m_dragMoved = false;
    qreal m_dragStartMouseX = 0;
    qreal m_dragStartCenterX = 0;
};

} // namespace autoflow
