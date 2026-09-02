#pragma once
#include <QWidget>

class QLabel;
class QMouseEvent;
class QContextMenuEvent;

namespace autoflow {

class IconButton;

// 无边框窗口的自定义标题栏：标题 + 最小化/最大化/关闭按钮 + 拖拽移动 + 双击最大化
class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget* parent = nullptr);

    void setTitle(const QString& title);

signals:
    void minimizeRequested();
    void maximizeRequested();
    void closeRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;   // 右键弹系统菜单
    bool eventFilter(QObject* obj, QEvent* event) override;     // 监听窗口最大化状态

private:
    void updateMaximizeIcon();

    QLabel* m_title = nullptr;
    IconButton* m_maxBtn = nullptr;
    QPoint m_dragOffset;   // 按下时鼠标相对窗口左上角的偏移
};

} // namespace autoflow
