#pragma once
#include <QWidget>

class QTimer;

namespace autoflow {

// 执行时的屏幕边框：全屏置顶、鼠标穿透、四周发光线框 + 呼吸动画
class RunBorder : public QWidget {
    Q_OBJECT
public:
    explicit RunBorder(QWidget* parent = nullptr);

    // 覆盖主屏并显示边框
    void showOnScreen();

protected:
    void paintEvent(QPaintEvent*) override;
    void hideEvent(QHideEvent*) override;

private:
    QTimer* m_timer = nullptr;
    qreal m_phase = 0.0;    // 呼吸动画相位
    qreal m_scale = 1.0;    // 屏幕 DPI 缩放比例（线宽/边距据此放大）
};

} // namespace autoflow
