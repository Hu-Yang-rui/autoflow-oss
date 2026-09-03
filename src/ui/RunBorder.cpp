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
    p.setRenderHint(QPainter::Antialiasing);
    const bool dark = ThemeManager::instance().isDark();
    const QColor base = Palette::accent(dark);   // 主题强调色（蓝）

    // 边框距屏幕边缘的距离（给光晕留出向内扩散的空间）
    const int m = 16;
    QRect r = rect().adjusted(m, m, -m, -m);

    // 光晕：向屏幕内侧多层扩散，透明度由浓到淡（越靠内越淡）
    const int N = 18;
    for (int i = 0; i < N; ++i) {
        int alpha = 72 - i * 4;                  // 72 → 4，逐层变淡
        QColor c = base;
        c.setAlpha(alpha);
        p.setPen(QPen(c, 2));
        p.drawRect(r.adjusted(i, i, -i, -i));
    }

    // 主体高亮边框（最外层，最亮）
    p.setPen(QPen(base.lighter(150), 3));
    p.drawRect(r);
}

} // namespace autoflow
