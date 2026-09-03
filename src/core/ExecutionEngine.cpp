#include "ExecutionEngine.h"
#include "Settings.h"
#include <QCoreApplication>
#include <set>

namespace autoflow {

namespace {
// 交互类指令（会移动光标/操作键盘）：静默运行时跳过
bool isInteractiveInstr(const std::string& id) {
    static const std::set<std::string> s = {
        "click", "move", "drag", "scroll", "keyboard", "keypress", "hotkey"
    };
    return s.count(id) > 0;
}
} // namespace

// ============================ EngineWorker ============================

EngineWorker::EngineWorker(QObject* parent) : QObject(parent) {}

QString EngineWorker::findStart() const {
    for (auto& n : m_flow.nodes) if (n.instr == "start") return QString::fromStdString(n.id);
    return m_flow.nodes.empty() ? QString() : QString::fromStdString(m_flow.nodes[0].id);
}

QString EngineWorker::nextNodeId(const FlowModel& flow, const FlowNode& node,
                                 const std::string& label, const std::string& jumpTarget) const {
    if (!jumpTarget.empty()) return QString::fromStdString(jumpTarget);
    if (label.empty()) return QString();   // end 指令
    const FlowEdge* e = flow.findOutEdge(node.id, label);
    if (e) return QString::fromStdString(e->to);
    auto outs = flow.outEdges(node.id);
    if (!outs.empty()) return QString::fromStdString(outs[0]->to);
    return QString();
}

QString EngineWorker::snapshotJson() const {
    return QString::fromStdString(m_vars.toJson().dump());
}

void EngineWorker::waitForResume() {
    QMutexLocker locker(&m_mutex);
    while (!m_resume && !m_stop.load()) m_cond.wait(&m_mutex);
    m_resume = false;
}

void EngineWorker::stop() {
    m_stop = true;
    QMutexLocker locker(&m_mutex);
    m_resume = true;
    m_cond.wakeAll();
}

void EngineWorker::stepOnce() {
    QMutexLocker locker(&m_mutex);
    m_resume = true;
    m_cond.wakeAll();
}

// 内嵌子流程执行（共享变量），返回是否成功
bool EngineWorker::runSubFlow(const json& body, VariableSystem& vars) {
    FlowModel sub;
    if (!sub.fromJson(body)) return false;

    QString cur = QString::fromStdString(sub.startNodeId());
    while (!cur.isEmpty()) {
        if (m_stop.load()) return false;
        const FlowNode* node = sub.nodeById(cur.toStdString());
        if (!node) return false;
        IInstruction* instr = InstructionRegistry::instance().get(node->instr);
        if (!instr) {
            emit logMessage("error", cur, "",
                            tr("未知指令: %1").arg(QString::fromStdString(node->instr)));
            return false;
        }
        QString nodeName = trInstr(instr->meta().name.c_str());

        // 禁用节点：跳过执行，直接沿第一个输出端口继续
        if (!node->enabled) {
            std::string skipLabel = instr->meta().outPorts.empty() ? "next" : instr->meta().outPorts[0];
            emit logMessage("info", QString::fromStdString(node->id), nodeName,
                            tr("已禁用，跳过该步骤"));
            if (node->instr == "end") return true;
            cur = nextNodeId(sub, *node, skipLabel, "");
            continue;
        }

        ExecutionContext ctx(vars);
        ctx.log = [this, id = node->id, nodeName](const std::string& lvl, const std::string& msg) {
            emit logMessage(QString::fromStdString(lvl), QString::fromStdString(id),
                            nodeName, QString::fromStdString(msg));
        };
        ctx.notifyVar = [this](const std::string&) { emit variablesSnapshot(snapshotJson()); };
        ctx.runSubFlow = [this, &vars](const json& b) { return runSubFlow(b, vars); };
        ctx.stopFlag = &m_stop;

        emit nodeStarted(QString::fromStdString(node->id), nodeName);
        qint64 t0 = QDateTime::currentMSecsSinceEpoch();
        std::string label = instr->execute(ctx, node->params);
        qint64 ms = QDateTime::currentMSecsSinceEpoch() - t0;
        emit nodeFinished(QString::fromStdString(node->id), ctx.error.empty(), ms,
                          QString::fromStdString(ctx.error));
        if (!ctx.error.empty()) {
            emit logMessage("error", QString::fromStdString(node->id), nodeName,
                            QString::fromStdString(ctx.error));
            return false;
        }
        if (node->instr == "end") break;
        cur = nextNodeId(sub, *node, label, ctx.jumpTarget);
    }
    return true;
}

void EngineWorker::runFlow(const QString& startNodeId) {
    m_stop = false;
    m_resume = false;
    m_vars.clear();
    for (auto& kv : m_flow.variables) m_vars.set(kv.first, kv.second);

    QString cur = startNodeId.isEmpty() ? findStart() : startNodeId;
    bool runOk = true;
    QString summary = tr("运行完成");
    int total = 0, failed = 0;
    const int maxSteps = Settings::instance().execMaxSteps();   // 0 = 不限

    while (!cur.isEmpty()) {
        if (m_stop.load()) { runOk = false; summary = tr("已手动停止"); break; }
        if (maxSteps > 0 && total >= maxSteps) {
            emit logMessage("warn", cur, "", tr("达到最大执行步数上限，已停止"));
            runOk = false;
            summary = tr("达到最大执行步数上限（%1 步），已停止").arg(maxSteps);
            break;
        }

        FlowNode* node = m_flow.nodeById(cur.toStdString());
        if (!node) {
            emit logMessage("error", cur, "", tr("节点不存在"));
            runOk = false; summary = tr("节点不存在"); break;
        }
        IInstruction* instr = InstructionRegistry::instance().get(node->instr);
        if (!instr) {
            emit logMessage("error", cur, "",
                            tr("未知指令: %1").arg(QString::fromStdString(node->instr)));
            runOk = false; summary = tr("未知指令"); break;
        }
        QString nodeName = trInstr(instr->meta().name.c_str());

        // 禁用节点：跳过执行，直接沿第一个输出端口继续
        if (!node->enabled) {
            std::string skipLabel = instr->meta().outPorts.empty() ? "next" : instr->meta().outPorts[0];
            emit logMessage("info", QString::fromStdString(node->id), nodeName,
                            tr("已禁用，跳过该步骤"));
            ++total;
            if (node->instr == "end") break;
            cur = nextNodeId(m_flow, *node, skipLabel, "");
            continue;
        }

        // 静默运行：跳过交互类指令（点击/输入等会碰光标），不打扰用户
        if (m_silent && isInteractiveInstr(node->instr)) {
            emit logMessage("warn", QString::fromStdString(node->id), nodeName,
                            tr("静默运行：已跳过该交互指令（不碰光标）"));
            ++total;
            if (node->instr == "end") break;
            cur = nextNodeId(m_flow, *node, "next", "");
            continue;
        }

        ExecutionContext ctx(m_vars);
        ctx.log = [this, id = node->id, nodeName](const std::string& lvl, const std::string& msg) {
            emit logMessage(QString::fromStdString(lvl), QString::fromStdString(id),
                            nodeName, QString::fromStdString(msg));
        };
        ctx.notifyVar = [this](const std::string&) { emit variablesSnapshot(snapshotJson()); };
        ctx.runSubFlow = [this](const json& b) { return runSubFlow(b, m_vars); };

        // 执行 + 错误重试
        bool nodeOk = false;
        QString nodeError;
        std::string label;
        int attempt = 0;
        while (true) {
            if (m_stepMode) waitForResume();
            if (m_stop.load()) break;

            ctx.error.clear();
            ctx.jumpTarget.clear();
            ctx.stopFlag = &m_stop;   // 实时读取停止状态（指针，每次循环都更新）

            emit nodeStarted(QString::fromStdString(node->id), nodeName);
            qint64 t0 = QDateTime::currentMSecsSinceEpoch();
            label = instr->execute(ctx, node->params);
            qint64 ms = QDateTime::currentMSecsSinceEpoch() - t0;

            nodeOk = ctx.error.empty();
            nodeError = QString::fromStdString(ctx.error);
            emit nodeFinished(QString::fromStdString(node->id), nodeOk, ms, nodeError);
            emit variablesSnapshot(snapshotJson());

            ++attempt;
            if (nodeOk) break;
            bool canRetry = (node->onError == OnError::Retry) && (attempt <= node->retry);
            if (!canRetry) break;
            emit logMessage("warn", QString::fromStdString(node->id), nodeName,
                            tr("第 %1 次失败，准备重试（%2/%3）").arg(attempt)
                                .arg(attempt + 1).arg(node->retry + 1));
        }

        ++total;
        if (!nodeOk) {
            ++failed;
            emit logMessage("error", QString::fromStdString(node->id), nodeName,
                            nodeError.isEmpty() ? tr("步骤执行失败") : nodeError);
            if (node->onError == OnError::Abort) {
                runOk = false;
                summary = tr("步骤失败中止: ") + nodeName;
                break;
            }
            // skip / retry 耗尽后继续
            emit logMessage("warn", QString::fromStdString(node->id), nodeName, tr("已跳过该步骤，继续执行"));
        }

        if (node->instr == "end") break;
        cur = nextNodeId(m_flow, *node, label, ctx.jumpTarget);
        if (cur.isEmpty() && !m_stop.load()) {
            emit logMessage("warn", QString::fromStdString(node->id), nodeName,
                            tr("该节点没有后续连线，流程结束"));
        }
    }

    summary += tr("（共 %1 步，失败 %2 步）").arg(total).arg(failed);
    emit variablesSnapshot(snapshotJson());
    emit runFinished(runOk, summary);
}

// ============================ ExecutionEngine ============================

ExecutionEngine::ExecutionEngine(QObject* parent) : QObject(parent) {
    m_worker = std::make_unique<EngineWorker>();

    connect(m_worker.get(), &EngineWorker::nodeStarted, this, &ExecutionEngine::nodeStarted);
    connect(m_worker.get(), &EngineWorker::nodeFinished, this, &ExecutionEngine::nodeFinished);
    connect(m_worker.get(), &EngineWorker::logMessage, this, &ExecutionEngine::logMessage);
    connect(m_worker.get(), &EngineWorker::variablesSnapshot, this, &ExecutionEngine::variablesSnapshot);
    connect(m_worker.get(), &EngineWorker::runFinished, this, &ExecutionEngine::onRunFinished);
}

ExecutionEngine::~ExecutionEngine() {
    if (m_worker) m_worker->stop();
    if (m_thread.joinable()) m_thread.join();
}

void ExecutionEngine::onRunFinished(bool ok, const QString& summary) {
    m_running = false;
    emit runningChanged(false);
    emit runFinished(ok, summary);
}

void ExecutionEngine::startRun(const QString& startNodeId) {
    if (m_running) return;
    if (m_thread.joinable()) m_thread.join();   // 清理上一次已结束的线程

    m_worker->setFlow(m_flow);
    m_worker->setStepMode(m_stepMode);
    m_worker->setSilent(m_silent);
    m_running = true;
    emit runningChanged(true);

    m_thread = std::thread([this, startNodeId]() {
        m_worker->runFlow(startNodeId);
    });
}

void ExecutionEngine::stop() {
    // 请求工作线程停止；运行状态将在 onRunFinished 中翻转为 false 并 emit runningChanged(false)
    if (m_worker) m_worker->stop();
}

void ExecutionEngine::stepOnce() {
    if (m_worker) m_worker->stepOnce();
}

} // namespace autoflow
