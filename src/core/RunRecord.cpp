#include "RunRecord.h"

namespace autoflow {

void RunRecord::clear() {
    startedAt = 0;
    finishedAt = 0;
    success = false;
    logs.clear();
    steps.clear();
}

void RunRecord::addLog(const QString& level, const QString& nodeId, const QString& nodeName,
                       const QString& message, qint64 elapsedMs) {
    RunLogEntry e;
    e.ts = QDateTime::currentMSecsSinceEpoch();
    e.level = level;
    e.nodeId = nodeId;
    e.nodeName = nodeName;
    e.message = message;
    e.elapsedMs = elapsedMs;
    logs.push_back(e);
}

void RunRecord::addStep(const RunStepResult& r) { steps.push_back(r); }

json RunRecord::toJson() const {
    json j;
    j["startedAt"] = (long long)startedAt;
    j["finishedAt"] = (long long)finishedAt;
    j["success"] = success;
    json ls = json::array();
    for (auto& e : logs) {
        ls.push_back({
            {"ts", (long long)e.ts}, {"level", e.level.toStdString()},
            {"nodeId", e.nodeId.toStdString()}, {"nodeName", e.nodeName.toStdString()},
            {"message", e.message.toStdString()}, {"elapsedMs", (long long)e.elapsedMs}
        });
    }
    j["logs"] = ls;
    json ss = json::array();
    for (auto& s : steps) {
        ss.push_back({
            {"nodeId", s.nodeId.toStdString()}, {"nodeName", s.nodeName.toStdString()},
            {"ok", s.ok}, {"elapsedMs", (long long)s.elapsedMs},
            {"error", s.error.toStdString()}, {"screenshot", s.screenshotPath.toStdString()}
        });
    }
    j["steps"] = ss;
    return j;
}

} // namespace autoflow
