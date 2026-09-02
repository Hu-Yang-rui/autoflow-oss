#pragma once
#include <QWidget>
#include <QString>
#include <QColor>
#include <QList>
#include <QElapsedTimer>

class QPropertyAnimation;
class QTimer;

namespace autoflow {

struct IslandParam {
    QString label;
    QString value;
};

// 系统级灵动岛：独立顶层无边框窗口，浮在屏幕最顶部居中
class DynamicIsland : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal expansion READ expansion WRITE setExpansion)
public:
    explicit DynamicIsland(QWidget* parent = nullptr);

    // progressMs > 0：显示确定性进度条（按实际时长从 0→100% 填充）
    // progressMs == 0：不显示进度条（非时间类模块）
    void setIdle();
    void setRunning(const QString& moduleName, const QList<IslandParam>& params = {},
                    int progressMs = 0);
    void setResult(const QString& summary);
    void setError(const QString& msg);

    qreal expansion() const { return m_expansion; }
    void setExpansion(qreal v);
    void reposition();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    void animateTo(qreal target);
    void updateSize();
    void onTick();

    qreal m_expansion = 0.0;
    QString m_moduleName;
    QList<IslandParam> m_params;
    QColor m_dotColor;
    QPropertyAnimation* m_anim = nullptr;
    QTimer* m_tickTimer = nullptr;
    QElapsedTimer m_stepElapsed;
    int m_progressTotal = 0;       // 进度条总时长（ms），0=不显示
};

} // namespace autoflow
