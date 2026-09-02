#include "TitleBar.h"
#include "IconButton.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QEvent>
#include <QWindow>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace autoflow {

TitleBar::TitleBar(QWidget* parent) : QWidget(parent) {
    setObjectName("titleBar");
    setFixedHeight(34);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 0, 0, 0);
    lay->setSpacing(0);

    m_title = new QLabel(QStringLiteral("AutoFlow"));
    m_title->setObjectName("titleBarTitle");
    lay->addWidget(m_title);
    lay->addStretch(1);

    auto makeBtn = [&](IconButton::Icon icon, const QString& tip, const char* objName) {
        auto* b = new IconButton(icon, this);
        b->setObjectName(objName);
        b->setFixedSize(46, 34);
        b->setToolTip(tip);
        lay->addWidget(b);
        return b;
    };

    auto* minBtn = makeBtn(IconButton::WinMinimize, tr("最小化"), "minBtn");
    m_maxBtn = makeBtn(IconButton::WinMaximize, tr("最大化"), "maxBtn");
    auto* closeBtn = makeBtn(IconButton::WinClose, tr("关闭"), "closeBtn");
    closeBtn->setDanger(true);

    connect(minBtn, &QAbstractButton::clicked, this, &TitleBar::minimizeRequested);
    connect(m_maxBtn, &QAbstractButton::clicked, this, &TitleBar::maximizeRequested);
    connect(closeBtn, &QAbstractButton::clicked, this, &TitleBar::closeRequested);

    // 监听窗口最大化/还原，切换按钮图标
    if (QWidget* pw = parentWidget())
        pw->installEventFilter(this);
    updateMaximizeIcon();
}

void TitleBar::setTitle(const QString& title) {
    m_title->setText(title);
}

void TitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && window()) {
        m_dragOffset = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent* event) {
    if ((event->buttons() & Qt::LeftButton) && window()) {
        if (window()->isMaximized()) {
            // 最大化状态下拖拽：先还原到普通窗口，再跟随鼠标
            window()->showNormal();
            m_dragOffset = QPoint(window()->width() / 2, 10);
        }
        window()->move(event->globalPosition().toPoint() - m_dragOffset);
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit maximizeRequested();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::updateMaximizeIcon() {
    if (!m_maxBtn) return;
    const bool max = window() && window()->isMaximized();
    m_maxBtn->setIconType(max ? IconButton::WinRestore : IconButton::WinMaximize);
    m_maxBtn->setToolTip(max ? tr("还原") : tr("最大化"));
}

bool TitleBar::eventFilter(QObject* obj, QEvent* event) {
    if (obj == parentWidget() && event->type() == QEvent::WindowStateChange)
        updateMaximizeIcon();
    return QWidget::eventFilter(obj, event);
}

void TitleBar::contextMenuEvent(QContextMenuEvent* event) {
#ifdef Q_OS_WIN
    // 无边框窗口：右键标题栏弹出系统菜单（还原/移动/大小/最小化/最大化/关闭）
    if (QWidget* w = window()) {
        HWND hwnd = reinterpret_cast<HWND>(w->winId());
        HMENU hMenu = GetSystemMenu(hwnd, FALSE);
        if (hMenu) {
            const QPoint gp = event->globalPos();
            SetForegroundWindow(hwnd);
            const int cmd = TrackPopupMenu(hMenu,
                TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON,
                gp.x(), gp.y(), 0, hwnd, nullptr);
            if (cmd)
                PostMessage(hwnd, WM_SYSCOMMAND, cmd, 0);
        }
    }
    event->accept();
#else
    QWidget::contextMenuEvent(event);
#endif
}

} // namespace autoflow
