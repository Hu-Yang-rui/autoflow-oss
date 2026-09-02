#include "CropOverlay.h"
#include "ThemeManager.h"
#include "Palette.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

namespace autoflow {

CropOverlay::CropOverlay(const QPixmap& background, QWidget* parent)
    : QWidget(parent), m_background(background) {
    // 无边框 + 置顶 + 工具窗口（不出现在任务栏）
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    // 全屏需要获得键盘焦点以响应 Esc
    setFocusPolicy(Qt::StrongFocus);
}

QRect CropOverlay::toPixmapRect(const QRect& widgetRect) const {
    if (width() <= 0 || height() <= 0 || m_background.isNull()) return widgetRect;
    double sx = (double)m_background.width() / width();
    double sy = (double)m_background.height() / height();
    return QRect(qRound(widgetRect.x() * sx), qRound(widgetRect.y() * sy),
                 qRound(widgetRect.width() * sx), qRound(widgetRect.height() * sy));
}

void CropOverlay::paintEvent(QPaintEvent* /*e*/) {
    QPainter p(this);
    const bool dark = ThemeManager::instance().effectiveDark();
    // 先铺满整屏截图（按控件尺寸缩放，兼容高 DPI）
    p.drawPixmap(rect(), m_background);

    // 整体暗化
    p.fillRect(rect(), Palette::scrim(dark));

    // 选区恢复清晰：重绘选区部分的截图，再描边
    if (!m_selection.isNull() && m_selection.isValid()) {
        QRect r = m_selection.normalized() & rect();
        if (!r.isEmpty()) {
            p.drawPixmap(r, m_background, toPixmapRect(r));
            p.setPen(QPen(Palette::accent(dark), 2));
            p.drawRect(r.adjusted(0, 0, -1, -1));
        }
    }

    // 顶部操作提示
    p.setPen(Qt::white);
    p.drawText(16, 32, tr("拖拽框选识别区域，松开确认，Esc 取消"));
}

void CropOverlay::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_origin = e->pos();
        m_selection = QRect(m_origin, m_origin);
        update();
    }
}

void CropOverlay::mouseMoveEvent(QMouseEvent* e) {
    if (m_dragging) {
        m_selection = QRect(m_origin, e->pos());
        update();
    }
}

void CropOverlay::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton || !m_dragging) return;
    m_dragging = false;
    QRect r = m_selection.normalized() & rect();
    // 选区过小视为误触，不确认
    if (r.width() < 4 || r.height() < 4) {
        m_selection = QRect();
        update();
        return;
    }
    m_result = toPixmapRect(r);
    emit regionSelected(m_result);
}

void CropOverlay::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape) {
        emit cancelled();
        return;
    }
    QWidget::keyPressEvent(e);
}

} // namespace autoflow
