#pragma once
#include <QObject>
#include <QTimer>
#include <QTime>
#include <QString>
#include <QVector>
#include <QDateTime>

namespace autoflow {

// 定时调度器（Phase 3）：每天/每周/间隔/指定时间，非阻塞
class Scheduler : public QObject {
    Q_OBJECT
public:
    enum Mode { Once, EveryInterval, Daily, Weekly };

    struct Task {
        QString name;
        QString flowPath;
        Mode mode = Once;
        int intervalSec = 60;      // EveryInterval
        QTime atTime;              // Once / Daily / Weekly
        QVector<int> weekDays;     // Weekly: 1=周一 .. 7=周日
        bool enabled = true;
        qint64 lastRun = 0;
    };

    explicit Scheduler(QObject* parent = nullptr);

    QVector<Task>& tasks() { return m_tasks; }
    void addTask(const Task& t) { m_tasks.push_back(t); }
    void removeTask(int idx);
    void setEnabled(bool on) { m_enabled = on; }
    bool isEnabled() const { return m_enabled; }

signals:
    void runTask(const QString& flowPath, const QString& name);
    void taskFired(const QString& name);

private slots:
    void onTick();

private:
    bool dueNow(const Task& t, const QDateTime& now);

    QVector<Task> m_tasks;
    QTimer* m_timer = nullptr;
    bool m_enabled = true;
};

} // namespace autoflow
