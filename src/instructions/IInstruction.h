#pragma once
#include "../common.h"
#include "../core/Variable.h"
#include "../core/Expression.h"

#include <QCoreApplication>
#include <atomic>
#include <QString>

namespace autoflow {

// i18n 辅助：指令名/描述/参数标签等元数据统一在 "Instructions" 上下文翻译。
// 定义处用 QT_TRANSLATE_NOOP("Instructions", ...) 标记（运行时恒等），
// 显示/记录处用 trInstr(...) 取译文；中文(默认)时 translate 返回源串，行为不变。
inline QString trInstr(const char* s) { return QCoreApplication::translate("Instructions", s); }

// 执行上下文：变量 + 日志回调 + 错误/跳转状态
struct ExecutionContext {
    VariableSystem& vars;
    std::function<void(const std::string& level, const std::string& msg)> log;
    std::function<void(const std::string& name)> notifyVar;
    std::string error;              // 非空表示该步失败
    std::string jumpTarget;         // 跳转指令设置目标节点 id
    const std::atomic<bool>* stopFlag = nullptr;   // 指向引擎的停止标志，实时读取
    // 运行内嵌子流程（JSON 描述的节点+连线），共享变量与日志；成功返回 true
    std::function<bool(const json& body)> runSubFlow;

    explicit ExecutionContext(VariableSystem& v) : vars(v) {}

    // 参数读取助手（自动做 ${...} 变量替换）
    std::string resolveString(const std::string& s) const { return autoflow::resolveString(s, vars); }

    std::string pStr(const json& p, const std::string& key, const std::string& def = "") const {
        if (!p.is_object() || !p.contains(key)) return resolveString(def);
        return resolveString(p[key].is_string() ? p[key].get<std::string>() : p[key].dump());
    }
    double pNum(const json& p, const std::string& key, double def = 0) const {
        if (!p.is_object() || !p.contains(key)) return def;
        Variable v;
        std::string raw = p[key].is_string() ? p[key].get<std::string>() : p[key].dump();
        if (autoflow::resolveValue(raw, vars, v) && v.type == VarType::Number) return v.num;
        try { return std::stod(raw); } catch (...) { return def; }
    }
    int pInt(const json& p, const std::string& key, int def = 0) const {
        return (int)pNum(p, key, def);
    }
    bool pBool(const json& p, const std::string& key, bool def = false) const {
        if (!p.is_object() || !p.contains(key)) return def;
        Variable v;
        std::string raw = p[key].is_string() ? p[key].get<std::string>() : p[key].dump();
        if (autoflow::resolveValue(raw, vars, v)) return v.isTruthy();
        return def;
    }

    void info(const std::string& msg)  { if (log) log("info", msg); }
    void ok(const std::string& msg)    { if (log) log("success", msg); }
    void warn(const std::string& msg)  { if (log) log("warn", msg); }
    void fail(const std::string& msg)  { if (log) log("error", msg); }
};

// 指令插件接口
struct IInstruction {
    virtual ~IInstruction() = default;

    struct Param {
        std::string key;
        std::string label;                 // 中文名
        std::string type;                  // string | int | number | bool | select | textarea
        std::string def;                   // 默认值
        std::vector<std::string> options;  // select 选项
        std::string hint;                  // 示例/说明
        bool required = true;

        Param() = default;
        Param(const std::string& k, const std::string& l, const std::string& t,
              const std::string& d, const std::string& h = "")
            : key(k), label(l), type(t), def(d), hint(h) {}
        Param& withOptions(std::initializer_list<std::string> o) { options.assign(o); return *this; }
        Param& opt() { required = false; return *this; }
    };

    struct Meta {
        std::string id;
        std::string category;              // 图像 | 键鼠 | 数据 | 流程 | AI
        std::string name;                  // 中文名
        std::string desc;                  // 简短说明
        std::vector<Param> params;
        std::vector<std::string> outPorts = { "next" };  // 输出端口标签
        bool hasInput = true;
    };

    virtual Meta meta() const = 0;
    // 返回下一步要走的连线标签（默认 "next"）。失败时设置 ctx.error 并返回空。
    virtual std::string execute(ExecutionContext& ctx, const json& params) = 0;
};

} // namespace autoflow
