#include "FlowModel.h"
#include "Settings.h"
#include <fstream>
#include <sstream>
#include <set>
#include <QCoreApplication>

namespace autoflow {

const FlowNode* FlowModel::nodeById(const std::string& id) const {
    for (auto& n : nodes) if (n.id == id) return &n;
    return nullptr;
}
FlowNode* FlowModel::nodeById(const std::string& id) {
    for (auto& n : nodes) if (n.id == id) return &n;
    return nullptr;
}
int FlowModel::indexOf(const std::string& id) const {
    for (size_t i = 0; i < nodes.size(); ++i) if (nodes[i].id == id) return (int)i;
    return -1;
}

void FlowModel::addNode(FlowNode n) {
    // 新建节点未显式设置重试次数时，应用全局默认重试（0 = 不应用，保持原行为）
    if (n.retry == 0) {
        const int defRetry = Settings::instance().execDefaultRetry();
        if (defRetry > 0) n.retry = defRetry;
    }
    nodes.push_back(std::move(n));
    m_dirty = true;
}

void FlowModel::removeNode(const std::string& id) {
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(),
        [&](const FlowNode& n) { return n.id == id; }), nodes.end());
    edges.erase(std::remove_if(edges.begin(), edges.end(),
        [&](const FlowEdge& e) { return e.from == id || e.to == id; }), edges.end());
    m_dirty = true;
}

void FlowModel::addEdge(FlowEdge e) { edges.push_back(std::move(e)); m_dirty = true; }

void FlowModel::removeEdge(const std::string& from, const std::string& label) {
    edges.erase(std::remove_if(edges.begin(), edges.end(),
        [&](const FlowEdge& e) { return e.from == from && e.label == label; }), edges.end());
    m_dirty = true;
}

std::vector<const FlowEdge*> FlowModel::outEdges(const std::string& id) const {
    std::vector<const FlowEdge*> out;
    for (auto& e : edges) if (e.from == id) out.push_back(&e);
    return out;
}

const FlowEdge* FlowModel::findOutEdge(const std::string& id, const std::string& label) const {
    for (auto& e : edges) if (e.from == id && e.label == label) return &e;
    return nullptr;
}

std::string FlowModel::startNodeId() const {
    for (auto& n : nodes) if (n.instr == "start") return n.id;
    return nodes.empty() ? "" : nodes[0].id;
}

std::string FlowModel::newId(const std::string& prefix) const {
    std::set<std::string> used;
    for (auto& n : nodes) used.insert(n.id);
    for (int i = 1; ; ++i) {
        std::string id = prefix + std::to_string(i);
        if (!used.count(id)) return id;
    }
}

json FlowModel::toJson() const {
    json j;
    j["name"] = name;
    j["settings"] = settings;

    json vars = json::object();
    for (auto& kv : variables) vars[kv.first] = kv.second.toJson();
    j["variables"] = vars;

    json ns = json::array();
    for (auto& n : nodes) {
        ns.push_back({
            {"id", n.id}, {"instr", n.instr}, {"params", n.params},
            {"x", n.x}, {"y", n.y}, {"onError", n.onError}, {"retry", n.retry},
            {"comment", n.comment}, {"enabled", n.enabled}
        });
    }
    j["nodes"] = ns;

    json es = json::array();
    for (auto& e : edges) es.push_back({ {"from", e.from}, {"to", e.to}, {"label", e.label} });
    j["edges"] = es;

    return j;
}

bool FlowModel::fromJson(const json& j) {
    if (!j.is_object()) return false;
    name = j.value("name", QCoreApplication::translate("FlowModel", "未命名流程").toStdString());
    settings = j.value("settings", json::object());

    variables.clear();
    if (j.contains("variables") && j["variables"].is_object())
        for (auto& [k, v] : j["variables"].items()) variables[k] = Variable::fromJson(v);

    nodes.clear();
    if (j.contains("nodes") && j["nodes"].is_array()) {
        for (auto& nj : j["nodes"]) {
            FlowNode n;
            n.id       = nj.value("id", "");
            n.instr    = nj.value("instr", "");
            n.params   = nj.value("params", json::object());
            n.x        = nj.value("x", 0.0);
            n.y        = nj.value("y", 0.0);
            n.onError  = nj.value("onError", OnError::Abort);
            n.retry    = nj.value("retry", 0);
            n.comment  = nj.value("comment", "");
            n.enabled  = nj.value("enabled", true);
            nodes.push_back(std::move(n));
        }
    }

    edges.clear();
    if (j.contains("edges") && j["edges"].is_array()) {
        for (auto& ej : j["edges"]) {
            FlowEdge e;
            e.from  = ej.value("from", "");
            e.to    = ej.value("to", "");
            e.label = ej.value("label", "next");
            edges.push_back(e);
        }
    }
    return true;
}

bool FlowModel::saveToFile(const std::string& path, std::string& err) const {
    try {
        std::ofstream f(path, std::ios::binary);
        if (!f) {
            err = QCoreApplication::translate("FlowModel", "无法写入文件: %1")
                      .arg(QString::fromStdString(path)).toStdString();
            return false;
        }
        f << toJson().dump(2);
        return true;
    } catch (const std::exception& e) { err = e.what(); return false; }
}

bool FlowModel::loadFromFile(const std::string& path, std::string& err) {
    try {
        std::ifstream f(path, std::ios::binary);
        if (!f) {
            err = QCoreApplication::translate("FlowModel", "无法打开文件: %1")
                      .arg(QString::fromStdString(path)).toStdString();
            return false;
        }
        std::stringstream ss; ss << f.rdbuf();
        json j = json::parse(ss.str());
        bool ok = fromJson(j);
        if (ok) m_dirty = false;
        return ok;
    } catch (const std::exception& e) { err = e.what(); return false; }
}

} // namespace autoflow
