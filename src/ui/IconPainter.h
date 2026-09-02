#pragma once
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QtMath>

namespace autoflow {

// 统一的圆角线性矢量图标库。
// 所有图标在 24×24 设计网格内定义，1.75px 描边、圆头圆角（RoundCap/RoundJoin），
// 通过 color 参数适配深浅色。UI 里所有图标统一走这里，保证同描边、同端点、同几何语言。
namespace IconPainter {

enum class Id {
    // 窗口控制
    WinMinimize, WinMaximize, WinRestore, WinClose,
    // 基础
    Plus, Minus, ChevronDown, ChevronRight,
    // 指令分类（图像/键鼠/数据/流程/AI/窗口/文件/系统）
    CatImage, CatInput, CatData, CatFlow, CatAI, CatWindow, CatFile, CatSystem,
    // 状态/动作
    Sun, Moon, Play, Stop,
};

inline void paint(QPainter& p, const QRectF& box, Id id, const QColor& color, qreal strokeW = 1.75) {
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);

    // 把 24×24 设计网格等比映射到目标 box（居中）；描边宽度按尺寸等比缩放
    const qreal s = qMin(box.width(), box.height()) / 24.0;
    const QPointF o = box.center() - QPointF(12.0 * s, 12.0 * s);
    p.setPen(QPen(color, strokeW * s, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    auto P = [&](qreal x, qreal y) { return QPointF(o.x() + x * s, o.y() + y * s); };
    auto L = [&](qreal x1, qreal y1, qreal x2, qreal y2) { p.drawLine(P(x1, y1), P(x2, y2)); };
    auto R = [&](qreal x, qreal y, qreal w, qreal h) { p.drawRect(QRectF(P(x, y), P(x + w, y + h))); };

    switch (id) {
    // ---- 窗口控制 ----
    case Id::WinMinimize:
        L(6, 17, 18, 17);
        break;
    case Id::WinMaximize:
        R(5.5, 5.5, 13, 13);
        break;
    case Id::WinRestore:
        // 两个叠置方框：后(左上) + 前(右下)
        R(4.5, 8.5, 11, 11);
        R(8.5, 4.5, 11, 11);
        break;
    case Id::WinClose:
        L(6.5, 6.5, 17.5, 17.5);
        L(17.5, 6.5, 6.5, 17.5);
        break;

    // ---- 基础 ----
    case Id::Plus:
        L(12, 7, 12, 17);
        L(7, 12, 17, 12);
        break;
    case Id::Minus:
        L(7, 12, 17, 12);
        break;
    case Id::ChevronDown: {
        QPainterPath path;
        path.moveTo(P(8, 10)); path.lineTo(P(12, 14)); path.lineTo(P(16, 10));
        p.drawPath(path);
        break;
    }
    case Id::ChevronRight: {
        QPainterPath path;
        path.moveTo(P(10, 8)); path.lineTo(P(14, 12)); path.lineTo(P(10, 16));
        p.drawPath(path);
        break;
    }

    // ---- 指令分类 ----
    case Id::CatImage: {
        R(4, 5, 16, 14);                                   // 图片框
        p.drawEllipse(P(9, 9), 2.2 * s, 2.2 * s);          // 太阳
        QPainterPath m;                                     // 山
        m.moveTo(P(5, 17)); m.lineTo(P(9, 13)); m.lineTo(P(12, 15)); m.lineTo(P(15, 12)); m.lineTo(P(19, 16));
        p.drawPath(m);
        break;
    }
    case Id::CatInput: {
        R(7, 5, 10, 14);      // 鼠标主体
        L(12, 5, 12, 9);      // 左右键分界
        L(12, 19, 12, 21);    // 线缆
        break;
    }
    case Id::CatData: {
        R(4.5, 5, 15, 14);    // 表格外框
        L(4.5, 10, 19.5, 10); // 横线 1
        L(4.5, 15, 19.5, 15); // 横线 2
        L(10, 10, 10, 19);    // 竖线
        break;
    }
    case Id::CatFlow: {
        QPainterPath d;       // 中心菱形
        d.moveTo(P(12, 6)); d.lineTo(P(16, 12)); d.lineTo(P(12, 18)); d.lineTo(P(8, 12)); d.closeSubpath();
        p.drawPath(d);
        L(4, 12, 8, 12);      // 左进
        L(16, 12, 20, 12);    // 右出
        break;
    }
    case Id::CatAI: {
        QPainterPath sp;      // 四角星
        sp.moveTo(P(12, 5)); sp.lineTo(P(14, 10)); sp.lineTo(P(19, 12)); sp.lineTo(P(14, 14));
        sp.lineTo(P(12, 19)); sp.lineTo(P(10, 14)); sp.lineTo(P(5, 12)); sp.lineTo(P(10, 10));
        sp.closeSubpath();
        p.drawPath(sp);
        break;
    }
    case Id::CatWindow: {
        R(4.5, 6, 15, 13);    // 窗口框
        L(4.5, 10, 19.5, 10); // 标题栏分隔
        break;
    }
    case Id::CatFile: {
        QPainterPath f;
        f.moveTo(P(7, 4)); f.lineTo(P(15, 4)); f.lineTo(P(18, 7));
        f.lineTo(P(18, 20)); f.lineTo(P(6, 20)); f.lineTo(P(6, 4)); f.closeSubpath();
        p.drawPath(f);
        L(15, 4, 15, 7); L(15, 7, 18, 7);   // 折角
        break;
    }
    case Id::CatSystem: {
        p.drawEllipse(P(12, 12), 4.5 * s, 4.5 * s);       // 齿轮中心
        for (int i = 0; i < 8; ++i) {                      // 8 个齿
            const qreal a = qDegreesToRadians(45.0 * i);
            const QPointF d(qCos(a), qSin(a));
            L(12 + d.x() * 6.5, 12 + d.y() * 6.5, 12 + d.x() * 8.2, 12 + d.y() * 8.2);
        }
        break;
    }

    // ---- 状态/动作 ----
    case Id::Sun: {
        p.drawEllipse(P(12, 12), 4.0 * s, 4.0 * s);       // 中心圆
        for (int i = 0; i < 8; ++i) {                      // 8 条光芒
            const qreal a = qDegreesToRadians(45.0 * i);
            const QPointF d(qCos(a), qSin(a));
            L(12 + d.x() * 7, 12 + d.y() * 7, 12 + d.x() * 10, 12 + d.y() * 10);
        }
        break;
    }
    case Id::Moon: {
        QPainterPath full, cut;                            // 新月：圆减去偏移圆
        full.addEllipse(P(12, 12), 7.0 * s, 7.0 * s);
        cut.addEllipse(P(9, 11), 7.0 * s, 7.0 * s);
        p.drawPath(full.subtracted(cut));
        break;
    }
    case Id::Play: {
        QPainterPath tri;
        tri.moveTo(P(9, 6)); tri.lineTo(P(18, 12)); tri.lineTo(P(9, 18)); tri.closeSubpath();
        p.drawPath(tri);
        break;
    }
    case Id::Stop:
        R(7, 7, 10, 10);
        break;
    }
    p.restore();
}

} // namespace IconPainter
} // namespace autoflow
