#pragma once
#include <QWidget>
#include <QPixmap>
#include <QRect>
#include <QPoint>

namespace autoflow {

// 全屏截图裁剪遮罩：覆盖整个屏幕，显示暗化的屏幕快照，
// 用户拖拽框选区域（选区保持清晰），松开确认，Esc 取消。
class CropOverlay : public QWidget {
    Q_OBJECT
public:
    // background 为全屏截图（屏幕物理像素，原点在 (0,0)）
    explicit CropOverlay(const QPixmap& background, QWidget* parent = nullptr);

    // 选中的矩形（屏幕坐标 / 截图像素坐标），取消时为空
    QRect selectedRect() const { return m_result; }

signals:
    void regionSelected(const QRect& rect); // 松开鼠标且选区有效
    void cancelled();                       // Esc 取消

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    // 控件逻辑坐标 → 截图像素坐标（高 DPI 下截图尺寸大于控件尺寸）
    QRect toPixmapRect(const QRect& widgetRect) const;

    QPixmap m_background; // 全屏截图
    QPoint  m_origin;     // 拖拽起点
    QRect   m_selection;  // 当前拖拽中的选区（控件坐标）
    QRect   m_result;     // 确认后的选区（屏幕坐标）
    bool    m_dragging = false;
};

} // namespace autoflow
