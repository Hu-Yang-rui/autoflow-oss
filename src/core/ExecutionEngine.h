#pragma once
#include <QObject>
#include <QString>
#include <QMutex>
#include <QWaitCondition>
#include <QDateTime>
#include <atomic>
#include <thread>
#include <memory>

#include "FlowModel.h"
#include "Variable.h"
#include "../instructions/IInstruction.h"
#include "../instructions/InstructionRegistry.h"

namespace autoflow {

// 流程执行工作线程体：可在 std::thread 中运行，通过信号回报进度（与 UI 解耦）
class EngineWorker : public QObject {
    Q_OBJECT
public:
    explicit EngineWorker(QObject* parent = nullptr);

    void setFlow(const FlowModel& flow) { m_flow = flow; }
    void setStepMode(bool on) { m_stepMode = on; }

public slots:
    void runFlow(const QString& startNodeId);
    void stop();
    void stepOnce();

signals:
    void nodeStarted(const QString& id, const QString& name);
    void nodeFinished(const QString& id, bool ok, qint64 ms, const QString& error);
    void logMessage(const QString& level, const QString& nodeId,
                    const QString& nodeName, const QString& text);
    void runFinished(bool ok, const QString& summary);
    void variablesSnapshot(const QString& json);

private:
    FlowModel m_flow;
    VariableSystem m_vars;
    std::atomic<bool> m_stop{false};
    bool m_stepMode = false;
    QMutex m_mutex;
    QWaitCondition m_cond;
    bool m_resume = false;

    QString findStart() const;
    QString nextNodeId(const FlowModel& flow, const FlowNode& node,
                       const std::string& label, const std::string& jumpTarget) const;
    bool runSubFlow(const json& body, VariableSystem& vars);
    void waitForResume();
    QString snapshotJson() const;
};

// 执行引擎门面：管理后台线程，UI 通过信号监听运行状态
class ExecutionEngine : public QObject {
    Q_OBJECT
public:
    explicit ExecutionEngine(QObject* parent = nullptr);
    ~ExecutionEngine();

    void setFlow(const FlowModel& flow) { m_flow = flow; }
    void setStepMode(bool on) { m_stepMode = on; }
    bool isRunning() const { return m_running; }

signals:
    void nodeStarted(const QString& id, const QString& name);
    void nodeFinished(const QString& id, bool ok, qint64 ms, const QString& error);
    void logMessage(const QString& level, const QString& nodeId,
                    const QString& nodeName, const QString& text);
    void runFinished(bool ok, const QString& summary);
    void variablesSnapshot(const QString& json);
    void runningChanged(bool running);   // 运行状态变化：true=开始，false=结束/停止

public slots:
    void startRun(const QString& startNodeId = QString());
    void stop();
    void stepOnce();

private slots:
    void onRunFinished(bool ok, const QString& summary);

private:
    FlowModel m_flow;
    bool m_stepMode = false;
    bool m_running = false;
    std::unique_ptr<EngineWorker> m_worker;
    std::thread m_thread;
};

} // namespace autoflow
