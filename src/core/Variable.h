#pragma once
#include "common.h"

namespace autoflow {

// 变量类型
enum class VarType : int { Null = 0, String, Number, Bool, List, Object };

// 统一变量值：字符串 / 数字 / 布尔 / 列表 / 对象
struct Variable {
    VarType type = VarType::Null;
    std::string str;
    double num = 0.0;
    bool boolean = false;
    std::vector<Variable> list;
    std::map<std::string, Variable> object;

    static Variable makeString(const std::string& s);
    static Variable makeNumber(double n);
    static Variable makeBool(bool b);
    static Variable makeList();
    static Variable makeObject();
    static Variable makeNull();

    bool isTruthy() const;
    std::string toString() const;           // 人类可读
    json toJson() const;                    // {"t": type, "v": value}
    static Variable fromJson(const json& j);

    // 子项访问：${obj.key} / ${list.0}
    bool getChild(const std::string& key, Variable& out) const;
    void setChild(const std::string& key, const Variable& v);
};

// 任意 JSON 值 → Variable（用于 JSON 解析 / HTTP 响应）
Variable variableFromJsonValue(const json& j);

// 全局变量 + 步骤返回值的存储（字符串/数字/列表/对象）
class VariableSystem {
public:
    void set(const std::string& name, const Variable& v);
    bool get(const std::string& name, Variable& out) const;
    bool has(const std::string& name) const;
    void remove(const std::string& name);
    void clear();

    const std::map<std::string, Variable>& all() const { return m_map; }
    std::map<std::string, Variable>& all() { return m_map; }

    json toJson() const;
    void fromJson(const json& j);

private:
    std::map<std::string, Variable> m_map;
};

} // namespace autoflow
