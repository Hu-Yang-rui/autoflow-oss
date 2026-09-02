#include "Expression.h"
#include <QCoreApplication>

namespace autoflow {

// 解析 ${a.b.c} 内部路径
static bool resolvePath(const std::string& path, const VariableSystem& vars, Variable& out) {
    auto parts = split(trim(path), '.');
    if (parts.empty()) return false;
    Variable cur;
    if (!vars.get(parts[0], cur)) return false;
    for (size_t i = 1; i < parts.size(); ++i) {
        Variable next;
        if (!cur.getChild(parts[i], next)) return false;
        cur = next;
    }
    out = cur;
    return true;
}

bool resolveValue(const std::string& exprIn, const VariableSystem& vars, Variable& out) {
    std::string expr = trim(exprIn);
    if (expr.empty()) { out = Variable::makeString(""); return true; }

    // 整串引用 ${...}
    if (expr.size() > 3 && expr.rfind("${", 0) == 0 && expr.back() == '}') {
        return resolvePath(expr.substr(2, expr.size() - 3), vars, out);
    }

    // 数字字面量
    if (expr.find_first_of("0123456789") == 0 || (expr.size() > 1 && expr[0] == '-' && expr[1] >= '0' && expr[1] <= '9')) {
        try {
            size_t pos = 0;
            double d = std::stod(expr, &pos);
            if (pos == expr.size()) { out = Variable::makeNumber(d); return true; }
        } catch (...) {}
    }

    // 布尔字面量
    if (expr == "true")  { out = Variable::makeBool(true);  return true; }
    if (expr == "false") { out = Variable::makeBool(false); return true; }

    // 引号字符串
    if (expr.size() >= 2 && ((expr.front() == '"' && expr.back() == '"') ||
                             (expr.front() == '\'' && expr.back() == '\''))) {
        out = Variable::makeString(expr.substr(1, expr.size() - 2));
        return true;
    }

    // 普通字符串字面量
    out = Variable::makeString(expr);
    return true;
}

std::string resolveString(const std::string& input, const VariableSystem& vars) {
    std::string out;
    size_t i = 0;
    while (i < input.size()) {
        size_t start = input.find("${", i);
        if (start == std::string::npos) { out += input.substr(i); break; }
        out += input.substr(i, start - i);
        size_t end = input.find('}', start + 2);
        if (end == std::string::npos) { out += input.substr(start); break; }
        std::string path = input.substr(start + 2, end - start - 2);
        Variable v;
        if (resolvePath(path, vars, v)) out += v.toString();
        // 找不到的引用替换为空串
        i = end + 1;
    }
    return out;
}

static bool isNumber(const Variable& v) { return v.type == VarType::Number; }

static bool cmp(const Variable& a, const Variable& b, const std::string& op) {
    if (op == "==") {
        if (isNumber(a) && isNumber(b)) return a.num == b.num;
        return a.toString() == b.toString();
    }
    if (op == "!=") {
        if (isNumber(a) && isNumber(b)) return a.num != b.num;
        return a.toString() != b.toString();
    }
    if (op == "contains") {
        return a.toString().find(b.toString()) != std::string::npos;
    }
    // 数值比较
    if (isNumber(a) && isNumber(b)) {
        if (op == ">")  return a.num >  b.num;
        if (op == "<")  return a.num <  b.num;
        if (op == ">=") return a.num >= b.num;
        if (op == "<=") return a.num <= b.num;
    } else {
        std::string sa = a.toString(), sb = b.toString();
        if (op == ">")  return sa >  sb;
        if (op == "<")  return sa <  sb;
        if (op == ">=") return sa >= sb;
        if (op == "<=") return sa <= sb;
    }
    return false;
}

bool evaluateCondition(const Condition& c, const VariableSystem& vars) {
    Variable a, b;
    resolveValue(c.left, vars, a);
    resolveValue(c.right, vars, b);
    return cmp(a, b, c.op);
}

bool evaluateConditionGroup(const json& group, const VariableSystem& vars, std::string& err) {
    if (!group.is_object() || !group.contains("conditions")) {
        err = QCoreApplication::translate("Expression", "条件配置缺少 conditions 字段").toStdString();
        return false;
    }
    std::string mode = group.value("mode", "all");
    const json& conds = group["conditions"];
    if (!conds.is_array() || conds.empty()) {
        err = QCoreApplication::translate("Expression", "条件列表为空").toStdString();
        return false;
    }

    bool result = (mode != "any");
    for (auto& cj : conds) {
        Condition c;
        c.left  = cj.value("left", "");
        c.op    = cj.value("op", "==");
        c.right = cj.value("right", "");
        bool r = evaluateCondition(c, vars);
        if (mode == "any") { if (r) { result = true; break; } }
        else               { if (!r) { result = false; break; } }
    }
    return result;
}

} // namespace autoflow
