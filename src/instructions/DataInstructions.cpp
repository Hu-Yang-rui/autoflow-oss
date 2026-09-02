#include "IInstruction.h"
#include "InstructionRegistry.h"
#include "../infra/HttpClient.h"
#include "../infra/InfraStubs.h"
#include <fstream>
#include <sstream>

namespace autoflow {

// ============================ 数据类指令 ============================

struct HttpInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "http"; m.category = Category::Data; m.name = QT_TRANSLATE_NOOP("Instructions", "HTTP 请求");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "发送 HTTP 请求并把响应保存到变量");
        m.params = {
            Param("method", QT_TRANSLATE_NOOP("Instructions", "请求方法"), "select", "GET",
                  QT_TRANSLATE_NOOP("Instructions", "GET/POST")).withOptions({ "GET", "POST", "PUT", "DELETE" }),
            Param("url", QT_TRANSLATE_NOOP("Instructions", "请求地址"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "例如 http://example.com/api")),
            Param("body", QT_TRANSLATE_NOOP("Instructions", "请求体"), "textarea", "",
                  QT_TRANSLATE_NOOP("Instructions", "POST/PUT 时使用")),
            Param("contentType", "Content-Type", "string", "application/json",
                  QT_TRANSLATE_NOOP("Instructions", "请求体类型")),
            Param("timeout", QT_TRANSLATE_NOOP("Instructions", "超时(毫秒)"), "int", "10000",
                  QT_TRANSLATE_NOOP("Instructions", "例如 10000 = 10 秒")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "响应保存到变量"), "string", "httpResult",
                  QT_TRANSLATE_NOOP("Instructions", "响应正文")),
            Param("saveStatus", QT_TRANSLATE_NOOP("Instructions", "状态码保存到变量"), "string", "httpStatus",
                  QT_TRANSLATE_NOOP("Instructions", "可留空"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string method = ctx.pStr(params, "method", "GET");
        std::string url = ctx.pStr(params, "url");
        std::string body = ctx.pStr(params, "body");
        std::string contentType = ctx.pStr(params, "contentType");
        int timeout = ctx.pInt(params, "timeout", 10000);
        if (url.empty()) { ctx.error = QCoreApplication::translate("Instructions", "请求地址为空").toStdString(); return ""; }

        HttpResponse res = HttpClient::request(method, url, body, contentType, timeout);
        if (!res.ok) {
            ctx.error = res.error.empty()
                ? QCoreApplication::translate("Instructions", "HTTP 请求失败").toStdString() : res.error;
            return "";
        }

        std::string saveVar = ctx.pStr(params, "saveVar", "httpResult");
        ctx.vars.set(saveVar, Variable::makeString(res.body));
        if (ctx.notifyVar) ctx.notifyVar(saveVar);

        std::string saveStatus = ctx.pStr(params, "saveStatus");
        if (!saveStatus.empty()) {
            ctx.vars.set(saveStatus, Variable::makeNumber(res.status));
            if (ctx.notifyVar) ctx.notifyVar(saveStatus);
        }
        ctx.info(QCoreApplication::translate("Instructions", "%1 %2 → %3（%4 字节）")
                     .arg(QString::fromStdString(method)).arg(QString::fromStdString(url))
                     .arg(res.status).arg((qulonglong)res.body.size()).toStdString());
        return "next";
    }
};

struct SetVarInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "setvar"; m.category = Category::Data; m.name = QT_TRANSLATE_NOOP("Instructions", "变量赋值");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "给变量赋值");
        m.params = {
            Param("name",  QT_TRANSLATE_NOOP("Instructions", "变量名"), "string", "result",
                  QT_TRANSLATE_NOOP("Instructions", "变量名")),
            Param("value", QT_TRANSLATE_NOOP("Instructions", "值"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量} 引用、数字、文本或 JSON")),
            Param("type",  QT_TRANSLATE_NOOP("Instructions", "类型"), "select", "auto",
                  QT_TRANSLATE_NOOP("Instructions", "自动推断类型")).withOptions({ "auto", "string", "number", "bool", "list", "object" })
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string name = ctx.pStr(params, "name");
        if (name.empty()) { ctx.error = QCoreApplication::translate("Instructions", "变量名为空").toStdString(); return ""; }
        std::string valueRaw = params.value("value", std::string(""));
        std::string type = params.value("type", "auto");

        std::string resolved = ctx.resolveString(valueRaw);
        Variable v;
        if (type == "string") {
            v = Variable::makeString(resolved);
        } else if (type == "number") {
            v = Variable::makeNumber(ctx.pNum(params, "value"));
        } else if (type == "bool") {
            v = Variable::makeBool(ctx.pBool(params, "value"));
        } else if (type == "list" || type == "object") {
            try { v = variableFromJsonValue(json::parse(resolved)); }
            catch (...) {
                ctx.error = QCoreApplication::translate("Instructions", "JSON 解析失败: %1")
                                .arg(QString::fromStdString(resolved)).toStdString();
                return "";
            }
        } else { // auto
            try {
                size_t pos = 0;
                double d = std::stod(resolved, &pos);
                if (pos == resolved.size() && !resolved.empty()) { v = Variable::makeNumber(d); }
                else if (resolved == "true") { v = Variable::makeBool(true); }
                else if (resolved == "false") { v = Variable::makeBool(false); }
                else if (!resolved.empty() && (resolved[0] == '[' || resolved[0] == '{'))
                    v = variableFromJsonValue(json::parse(resolved));
                else v = Variable::makeString(resolved);
            } catch (...) { v = Variable::makeString(resolved); }
        }
        ctx.vars.set(name, v);
        if (ctx.notifyVar) ctx.notifyVar(name);
        ctx.info(QCoreApplication::translate("Instructions", "变量 %1 = %2")
                     .arg(QString::fromStdString(name))
                     .arg(QString::fromStdString(v.toString())).toStdString());
        return "next";
    }
};

struct JsonParseInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "jsonparse"; m.category = Category::Data; m.name = QT_TRANSLATE_NOOP("Instructions", "JSON 解析");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "把 JSON 文本解析为对象/列表变量");
        m.params = {
            Param("source",  QT_TRANSLATE_NOOP("Instructions", "JSON 文本"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量} 引用")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "保存到变量"), "string", "jsonData",
                  QT_TRANSLATE_NOOP("Instructions", "解析结果"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string src = ctx.pStr(params, "source");
        std::string saveVar = ctx.pStr(params, "saveVar", "jsonData");
        try {
            Variable v = variableFromJsonValue(json::parse(src));
            ctx.vars.set(saveVar, v);
            if (ctx.notifyVar) ctx.notifyVar(saveVar);
            ctx.info(QCoreApplication::translate("Instructions", "JSON 已解析到变量 %1")
                         .arg(QString::fromStdString(saveVar)).toStdString());
            return "next";
        } catch (...) { ctx.error = QCoreApplication::translate("Instructions", "JSON 解析失败").toStdString(); return ""; }
    }
};

struct TextInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "text"; m.category = Category::Data; m.name = QT_TRANSLATE_NOOP("Instructions", "文本处理");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "拼接 / 替换 / 截取文本");
        m.params = {
            // 选项值会存入流程 JSON 并参与运行时比较，属于数据，不翻译
            Param("operation", QT_TRANSLATE_NOOP("Instructions", "操作"), "select", "拼接", "").withOptions({ "拼接", "替换", "截取", "转大写", "转小写" }),
            Param("input", QT_TRANSLATE_NOOP("Instructions", "输入文本"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("arg1", QT_TRANSLATE_NOOP("Instructions", "参数1"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "替换时=查找内容；截取时=起始位置")),
            Param("arg2", QT_TRANSLATE_NOOP("Instructions", "参数2"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "替换时=替换为；截取时=长度")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "保存到变量"), "string", "textResult", "")
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string op = ctx.pStr(params, "operation", "拼接");
        std::string input = ctx.pStr(params, "input");
        std::string a1 = ctx.pStr(params, "arg1");
        std::string a2 = ctx.pStr(params, "arg2");
        std::string saveVar = ctx.pStr(params, "saveVar", "textResult");
        std::string result = input;

        if (op == "拼接") result = input + a1 + a2;
        else if (op == "替换") {
            size_t pos = 0;
            while ((pos = result.find(a1, pos)) != std::string::npos) {
                result.replace(pos, a1.size(), a2);
                pos += a2.size();
            }
        } else if (op == "截取") {
            int start = 0, len = (int)input.size();
            try { start = std::stoi(a1); } catch (...) {}
            try { if (!a2.empty()) len = std::stoi(a2); } catch (...) {}
            result = input.substr(std::max(0, start), (size_t)std::max(0, len));
        } else if (op == "转大写") std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        else if (op == "转小写") std::transform(result.begin(), result.end(), result.begin(), ::tolower);

        ctx.vars.set(saveVar, Variable::makeString(result));
        if (ctx.notifyVar) ctx.notifyVar(saveVar);
        ctx.info(QCoreApplication::translate("Instructions", "文本处理结果已保存到 %1")
                     .arg(QString::fromStdString(saveVar)).toStdString());
        return "next";
    }
};

// 极简 CSV 写入/读取（无需第三方库）
static std::string csvEscape(const std::string& s) {
    if (s.find_first_of(",\"\n") == std::string::npos) return s;
    std::string o = "\"";
    for (char c : s) { if (c == '"') o += "\"\""; else o += c; }
    return o + "\"";
}

static std::vector<std::vector<std::string>> parseCsvText(const std::string& text) {
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> lines = split(text, '\n');
    for (auto& line : lines) {
        if (trim(line).empty()) continue;
        rows.push_back(split(line, ','));
    }
    return rows;
}

struct CsvInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "csv"; m.category = Category::Data; m.name = QT_TRANSLATE_NOOP("Instructions", "CSV 读写");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "读取或写入 CSV 文件");
        m.params = {
            // operation 选项值会存入流程 JSON 并参与运行时比较，属于数据，不翻译
            Param("operation", QT_TRANSLATE_NOOP("Instructions", "操作"), "select", "写入", "").withOptions({ "写入", "读取" }),
            Param("path", QT_TRANSLATE_NOOP("Instructions", "文件路径"), "string", "data.csv", ""),
            Param("data", QT_TRANSLATE_NOOP("Instructions", "数据"), "textarea", "标题,内容\n第1行,hello",
                  QT_TRANSLATE_NOOP("Instructions", "每行一条记录，逗号分隔")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "变量名"), "string", "csvData",
                  QT_TRANSLATE_NOOP("Instructions", "读取结果保存为列表"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string op = ctx.pStr(params, "operation", "写入");
        std::string path = ctx.pStr(params, "path", "data.csv");
        std::string data = ctx.pStr(params, "data");
        std::string saveVar = ctx.pStr(params, "saveVar", "csvData");

        if (op == "写入") {
            auto rows = parseCsvText(data);
            std::ofstream f(path);
            if (!f) {
                ctx.error = QCoreApplication::translate("Instructions", "无法写入 CSV: %1")
                                .arg(QString::fromStdString(path)).toStdString();
                return "";
            }
            for (auto& r : rows) {
                for (size_t i = 0; i < r.size(); ++i) {
                    if (i) f << ",";
                    f << csvEscape(r[i]);
                }
                f << "\n";
            }
            ctx.info(QCoreApplication::translate("Instructions", "CSV 已写入 %1")
                         .arg(QString::fromStdString(path)).toStdString());
        } else {
            std::ifstream f(path);
            if (!f) {
                ctx.error = QCoreApplication::translate("Instructions", "无法读取 CSV: %1")
                                .arg(QString::fromStdString(path)).toStdString();
                return "";
            }
            std::stringstream ss; ss << f.rdbuf();
            auto rows = parseCsvText(ss.str());
            Variable list = Variable::makeList();
            for (auto& r : rows) {
                Variable row = Variable::makeList();
                for (auto& c : r) row.list.push_back(Variable::makeString(c));
                list.list.push_back(row);
            }
            ctx.vars.set(saveVar, list);
            if (ctx.notifyVar) ctx.notifyVar(saveVar);
            ctx.info(QCoreApplication::translate("Instructions", "CSV 已读取到变量 %1（%2 行）")
                         .arg(QString::fromStdString(saveVar)).arg((qulonglong)rows.size()).toStdString());
        }
        return "next";
    }
};

struct ExcelInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "excel"; m.category = Category::Data; m.name = QT_TRANSLATE_NOOP("Instructions", "写入 Excel");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "把数据写入 .xlsx 文件");
        m.params = {
            Param("path",  QT_TRANSLATE_NOOP("Instructions", "Excel 路径"), "string", "output.xlsx", ""),
            Param("sheet", QT_TRANSLATE_NOOP("Instructions", "工作表名"), "string", "Sheet1", ""),
            Param("data",  QT_TRANSLATE_NOOP("Instructions", "数据"), "textarea", "姓名,得分\n张三,90\n李四,85",
                  QT_TRANSLATE_NOOP("Instructions", "每行一条记录，逗号分隔"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string path = ctx.pStr(params, "path", "output.xlsx");
        std::string sheet = ctx.pStr(params, "sheet", "Sheet1");
        std::string data = ctx.pStr(params, "data");
        auto rows = parseCsvText(data);
        std::string err;
        if (!excelWriteRows(path, sheet, rows, err)) { ctx.error = err; return ""; }
        ctx.info(QCoreApplication::translate("Instructions", "Excel 已写入 %1")
                     .arg(QString::fromStdString(path)).toStdString());
        return "next";
    }
};

void registerDataInstructions() {
    registerInstruction(std::make_unique<HttpInstr>());
    registerInstruction(std::make_unique<SetVarInstr>());
    registerInstruction(std::make_unique<JsonParseInstr>());
    registerInstruction(std::make_unique<TextInstr>());
    registerInstruction(std::make_unique<CsvInstr>());
    registerInstruction(std::make_unique<ExcelInstr>());
}

} // namespace autoflow
