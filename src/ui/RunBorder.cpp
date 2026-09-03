#include "RunBorder.h"
#include "Palette.h"
#include "ThemeManager.h"

#include <QPainter>
#include <QApplication>
#include <QScreen>

namespace autoflow {

RunBorder::RunBorder(QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);   // 鼠标穿透，不挡住任何操作
    setAttribute(Qt::WA_ShowWithoutActivating);
}

void RunBorder::showOnScreen() {
    if (QScreen* s = QApplication::primaryScreen()) {
        setGeometry(s->geometry());
    }
    show();
    raise();
}

void RunBorder::paintEvent(QPaintEvent*) {
    QPainter p(this);
    const bool dark = ThemeManager::instance().isDark();
    QColor accent = Palette::accent(dark);   // 主题强调色（蓝）

    // 外层光晕（发光）
    QColor glow = accent;
    glow.setAlpha(60);
    p.setPen(QPen(glow, 9));
    p.drawRect(rect().adjusted(4, 4, -5, -5));

    // 内层主体边框
    p.setPen(QPen(accent, 3));
    p.drawRect(rect().adjusted(1, 1, -2, -2));
}

} // namespace autoflow
