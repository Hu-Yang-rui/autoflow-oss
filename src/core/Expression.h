#pragma once
#include "Variable.h"

namespace autoflow {

// 解析 ${var} / ${obj.key} / ${list.0} 引用
// expr 形如:  "abc ${name} def"、纯 "${name}"、或字面量
bool resolveValue(const std::string& expr, const VariableSystem& vars, Variable& out);
std::string resolveString(const std::string& input, const VariableSystem& vars);

// 单个条件
struct Condition {
    std::string left;
    std::string op;   // == != > < >= <= contains
    std::string right;
};

bool evaluateCondition(const Condition& c, const VariableSystem& vars);
// 组合条件: {"mode":"all|any","conditions":[{left,op,right}]}
bool evaluateConditionGroup(const json& group, const VariableSystem& vars, std::string& err);

} // namespace autoflow
