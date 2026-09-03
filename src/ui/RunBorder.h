#pragma once
#include <QWidget>

namespace autoflow {

// 执行时的屏幕边框：全屏置顶、鼠标穿透、四周发光线框
class RunBorder : public QWidget {
    Q_OBJECT
public:
    explicit RunBorder(QWidget* parent = nullptr);

    // 覆盖主屏并显示边框
    void showOnScreen();

protected:
    void paintEvent(QPaintEvent*) override;
};

} // namespace autoflow
