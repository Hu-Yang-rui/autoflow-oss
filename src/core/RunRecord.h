#pragma once
#include <QString>
#include <QVector>
#include <QDateTime>
#include "common.h"

namespace autoflow {

// 单条运行日志
struct RunLogEntry {
    qint64 ts = 0;                 // 毫秒时间戳
    QString level;                 // info | success | warn | error
    QString nodeId;
    QString nodeName;
    QString message;
    qint64 elapsedMs = 0;
};

// 单步执行结果（用于运行记录与回放）
struct RunStepResult {
    QString nodeId;
    QString nodeName;
    bool ok = false;
    qint64 elapsedMs = 0;
    QString error;
    QString screenshotPath;        // Phase 2：关键步骤截图
};

// 一次运行的完整记录
class RunRecord {
public:
    qint64 startedAt = 0;
    qint64 finishedAt = 0;
    bool success = false;
    QVector<RunLogEntry> logs;
    QVector<RunStepResult> steps;

    void clear();
    void addLog(const QString& level, const QString& nodeId, const QString& nodeName,
                const QString& message, qint64 elapsedMs = 0);
    void addStep(const RunStepResult& r);
    json toJson() const;
};

} // namespace autoflow
