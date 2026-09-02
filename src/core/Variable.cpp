#include "Variable.h"

namespace autoflow {

Variable Variable::makeString(const std::string& s) { Variable v; v.type = VarType::String; v.str = s; return v; }
Variable Variable::makeNumber(double n)            { Variable v; v.type = VarType::Number; v.num = n; return v; }
Variable Variable::makeBool(bool b)                { Variable v; v.type = VarType::Bool; v.boolean = b; return v; }
Variable Variable::makeList()                      { Variable v; v.type = VarType::List; return v; }
Variable Variable::makeObject()                    { Variable v; v.type = VarType::Object; return v; }
Variable Variable::makeNull()                      { Variable v; v.type = VarType::Null; return v; }

bool Variable::isTruthy() const {
    switch (type) {
        case VarType::Null:   return false;
        case VarType::Bool:   return boolean;
        case VarType::Number: return num != 0.0;
        case VarType::String: return !str.empty();
        case VarType::List:   return !list.empty();
        case VarType::Object: return !object.empty();
    }
    return false;
}

std::string Variable::toString() const {
    switch (type) {
        case VarType::Null:   return "";
        case VarType::String: return str;
        case VarType::Number: return fmtNumber(num);
        case VarType::Bool:   return boolean ? "true" : "false";
        case VarType::List: {
            std::string s = "[";
            for (size_t i = 0; i < list.size(); ++i) {
                if (i) s += ", ";
                s += list[i].toString();
            }
            return s + "]";
        }
        case VarType::Object: {
            std::string s = "{";
            bool first = true;
            for (auto& kv : object) {
                if (!first) s += ", ";
                first = false;
                s += kv.first + ": " + kv.second.toString();
            }
            return s + "}";
        }
    }
    return "";
}

json Variable::toJson() const {
    switch (type) {
        case VarType::Null:   return json::object();
        case VarType::String: return { {"t","string"}, {"v", str} };
        case VarType::Number: return { {"t","number"}, {"v", num} };
        case VarType::Bool:   return { {"t","bool"},   {"v", boolean} };
        case VarType::List: {
            json arr = json::array();
            for (auto& e : list) arr.push_back(e.toJson());
            return { {"t","list"}, {"v", arr} };
        }
        case VarType::Object: {
            json obj = json::object();
            for (auto& kv : object) obj[kv.first] = kv.second.toJson();
            return { {"t","object"}, {"v", obj} };
        }
    }
    return json::object();
}

Variable Variable::fromJson(const json& j) {
    Variable v;
    if (!j.is_object()) { v = makeString(j.is_string() ? j.get<std::string>() : j.dump()); return v; }
    std::string t = j.value("t", "string");
    if (t == "string") { v = makeString(j.value("v", "")); }
    else if (t == "number") { v = makeNumber(j.value("v", 0.0)); }
    else if (t == "bool")   { v = makeBool(j.value("v", false)); }
    else if (t == "list") {
        v = makeList();
        if (j.contains("v") && j["v"].is_array())
            for (auto& e : j["v"]) v.list.push_back(fromJson(e));
    } else if (t == "object") {
        v = makeObject();
        if (j.contains("v") && j["v"].is_object())
            for (auto& [k, val] : j["v"].items()) v.object[k] = fromJson(val);
    }
    return v;
}

bool Variable::getChild(const std::string& key, Variable& out) const {
    if (type == VarType::Object) {
        auto it = object.find(key);
        if (it == object.end()) return false;
        out = it->second;
        return true;
    }
    if (type == VarType::List) {
        try {
            size_t idx = (size_t)std::stoll(key);
            if (idx >= list.size()) return false;
            out = list[idx];
            return true;
        } catch (...) { return false; }
    }
    return false;
}

void Variable::setChild(const std::string& key, const Variable& v) {
    if (type != VarType::Object && type != VarType::List) {
        type = VarType::Object;
        object.clear();
    }
    if (type == VarType::Object) {
        object[key] = v;
    } else if (type == VarType::List) {
        try {
            size_t idx = (size_t)std::stoll(key);
            if (idx >= list.size()) list.resize(idx + 1);
            list[idx] = v;
        } catch (...) {}
    }
}

Variable variableFromJsonValue(const json& j) {
    if (j.is_string()) return Variable::makeString(j.get<std::string>());
    if (j.is_number_integer() || j.is_number_float()) return Variable::makeNumber(j.get<double>());
    if (j.is_boolean()) return Variable::makeBool(j.get<bool>());
    if (j.is_null()) return Variable::makeNull();
    if (j.is_array()) {
        Variable v = Variable::makeList();
        for (auto& e : j) v.list.push_back(variableFromJsonValue(e));
        return v;
    }
    if (j.is_object()) {
        Variable v = Variable::makeObject();
        for (auto& [k, val] : j.items()) v.object[k] = variableFromJsonValue(val);
        return v;
    }
    return Variable::makeNull();
}

// ---------------------- VariableSystem ----------------------

void VariableSystem::set(const std::string& name, const Variable& v) { m_map[name] = v; }

bool VariableSystem::get(const std::string& name, Variable& out) const {
    auto it = m_map.find(name);
    if (it == m_map.end()) return false;
    out = it->second;
    return true;
}

bool VariableSystem::has(const std::string& name) const { return m_map.count(name) != 0; }
void VariableSystem::remove(const std::string& name) { m_map.erase(name); }
void VariableSystem::clear() { m_map.clear(); }

json VariableSystem::toJson() const {
    json j = json::object();
    for (auto& kv : m_map) j[kv.first] = kv.second.toJson();
    return j;
}

void VariableSystem::fromJson(const json& j) {
    clear();
    if (!j.is_object()) return;
    for (auto& [k, val] : j.items()) m_map[k] = Variable::fromJson(val);
}

} // namespace autoflow
