#include "Scheduler.h"

namespace autoflow {

Scheduler::Scheduler(QObject* parent) : QObject(parent) {
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);   // 每秒检查一次，不阻塞界面
    connect(m_timer, &QTimer::timeout, this, &Scheduler::onTick);
}

void Scheduler::removeTask(int idx) {
    if (idx >= 0 && idx < m_tasks.size()) m_tasks.removeAt(idx);
}

bool Scheduler::dueNow(const Task& t, const QDateTime& now) {
    if (!t.enabled) return false;
    qint64 nowMs = now.toMSecsSinceEpoch();
    switch (t.mode) {
        case EveryInterval: {
            if (t.lastRun == 0) return true;
            return (nowMs - t.lastRun) >= (qint64)t.intervalSec * 1000;
        }
        case Daily:
        case Once: {
            if (t.atTime.isValid()) {
                QDateTime due(now.date(), t.atTime);
                if (t.mode == Once) {
                    if (t.lastRun != 0) return false;
                    return nowMs >= due.toMSecsSinceEpoch();
                }
                return now.time().hour() == t.atTime.hour() &&
                       now.time().minute() == t.atTime.minute() &&
                       now.time().second() == t.atTime.second();
            }
            return false;
        }
        case Weekly: {
            int dow = now.date().dayOfWeek();  // 1=周一
            bool matchDay = t.weekDays.contains(dow);
            return matchDay && now.time().hour() == t.atTime.hour() &&
                   now.time().minute() == t.atTime.minute() &&
                   now.time().second() == t.atTime.second();
        }
    }
    return false;
}

void Scheduler::onTick() {
    if (!m_enabled) return;
    QDateTime now = QDateTime::currentDateTime();
    for (auto& t : m_tasks) {
        if (dueNow(t, now)) {
            t.lastRun = now.toMSecsSinceEpoch();
            if (t.mode == Once) t.enabled = false;
            emit taskFired(t.name);
            emit runTask(t.flowPath, t.name);
        }
    }
}

} // namespace autoflow
