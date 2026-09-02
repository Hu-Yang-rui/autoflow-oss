#pragma once
#include "common.h"
#include "Variable.h"

namespace autoflow {

// 流程节点（步骤）
struct FlowNode {
    std::string id;
    std::string instr;                 // 指令 id（注册表）
    json params = json::object();
    double x = 0, y = 0;
    std::string onError = OnError::Abort;   // abort | skip | retry
    int retry = 0;                          // 重试次数
    std::string comment;                    // 备注
    bool enabled = true;                    // 禁用后运行时不执行（直接跳过）
    std::string runState = "none";          // 运行时状态：none | running | ok | error（瞬态，不持久化）
};

// 连线（带标签，支持 真/假 分支）
struct FlowEdge {
    std::string from;
    std::string to;
    std::string label = "next";
};

// 工程：节点 + 连线 + 全局变量 + 设置
class FlowModel {
public:
    std::string name;
    std::vector<FlowNode> nodes;
    std::vector<FlowEdge> edges;
    std::map<std::string, Variable> variables;   // 全局变量初始值
    json settings = json::object();

    const FlowNode* nodeById(const std::string& id) const;
    FlowNode* nodeById(const std::string& id);
    int indexOf(const std::string& id) const;

    void addNode(FlowNode n);
    void removeNode(const std::string& id);      // 同时删除相关连线
    void addEdge(FlowEdge e);
    void removeEdge(const std::string& from, const std::string& label);

    std::vector<const FlowEdge*> outEdges(const std::string& id) const;
    const FlowEdge* findOutEdge(const std::string& id, const std::string& label) const;

    std::string startNodeId() const;             // 第一个 start 节点，否则第一个节点
    std::string newId(const std::string& prefix = "n") const;

    json toJson() const;
    bool fromJson(const json& j);
    bool saveToFile(const std::string& path, std::string& err) const;
    bool loadFromFile(const std::string& path, std::string& err);

    // 未保存保护：任何模型改动后置为 dirty；保存/载入时由调用方复位
    void setDirty(bool d) { m_dirty = d; }
    bool isDirty() const { return m_dirty; }

private:
    bool m_dirty = false;
};

} // namespace autoflow
