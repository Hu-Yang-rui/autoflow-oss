#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>

#include "IInstruction.h"
#include "InstructionRegistry.h"
#include "../core/FlowModel.h"

#include <thread>
#include <chrono>

namespace autoflow {

// ============================ 流程类指令 ============================

struct StartInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "start"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "开始");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "流程入口，每个流程从「开始」节点启动");
        m.hasInput = false; m.outPorts = { "next" };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json&) override {
        ctx.info(QCoreApplication::translate("Instructions", "流程开始").toStdString());
        return "next";
    }
};

struct EndInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "end"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "结束");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "流程结束，停止执行");
        m.outPorts = {};
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json&) override {
        ctx.info(QCoreApplication::translate("Instructions", "流程结束").toStdString());
        return "";   // 空 = 停止
    }
};

struct DelayInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "delay"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "延时");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "等待指定毫秒数");
        m.params = { Param("ms", QT_TRANSLATE_NOOP("Instructions", "延时(毫秒)"), "int", "1000",
                           QT_TRANSLATE_NOOP("Instructions", "例如 1000 = 1 秒")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int ms = ctx.pInt(params, "ms", 1000);
        if (ms < 0) ms = 0;
        ctx.info(QCoreApplication::translate("Instructions", "等待 %1 毫秒").arg(ms).toStdString());
        // 分段 sleep：每 50ms 检查一次停止标志（实时读取原子变量），确保点停止后能立即中断
        const int chunk = 50;
        for (int elapsed = 0; elapsed < ms; elapsed += chunk) {
            if (ctx.stopFlag && ctx.stopFlag->load()) {
                ctx.error = QCoreApplication::translate("Instructions", "已手动停止").toStdString();
                return "";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(std::min(chunk, ms - elapsed)));
        }
        return "next";
    }
};

// 检查指定名称的进程是否在运行（忽略大小写，进程名不带 .exe 时自动补）
static bool isProcessRunning(const std::string& name) {
    QString qname = QString::fromStdString(name);
    if (!qname.endsWith(".exe", Qt::CaseInsensitive)) qname += ".exe";
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (QString::fromWCharArray(pe.szExeFile).compare(qname, Qt::CaseInsensitive) == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

// 把中文比较符映射回内部符号（旧流程文件存的是 "==" 等，直接透传）
static std::string normalizeOp(const std::string& op) {
    if (op == "等于")     return "==";
    if (op == "不等于")   return "!=";
    if (op == "大于")     return ">";
    if (op == "小于")     return "<";
    if (op == "大于等于") return ">=";
    if (op == "小于等于") return "<=";
    if (op == "包含")     return "contains";
    return op;   // 旧值 "==" / "!=" / ">" / "<" / ">=" / "<=" / "contains" 直接透传
}

struct IfInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "if"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "条件判断");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "根据条件走「真」或「假」两条分支");
        m.params = {
            Param("preset", QT_TRANSLATE_NOOP("Instructions", "快捷预设"), "select", "自定义",
                  QT_TRANSLATE_NOOP("Instructions", "新手选预设即可；选「软件已打开/未打开」时在「比较的值」填进程名"))
                .withOptions({ "自定义", "找图成功", "找图失败", "AI 识别到目标", "AI 未识别到目标",
                               "OCR 识别到文字", "软件已打开", "软件未打开" }),
            Param("left",  QT_TRANSLATE_NOOP("Instructions", "变量"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "要判断的变量，格式 ${变量名}，如 ${count}、${match.found}")),
            Param("op",    QT_TRANSLATE_NOOP("Instructions", "条件"), "select", "等于",
                  QT_TRANSLATE_NOOP("Instructions", "等于/不等于/大于/小于/包含"))
                .withOptions({ "等于", "不等于", "大于", "小于", "大于等于", "小于等于", "包含" }),
            Param("right", QT_TRANSLATE_NOOP("Instructions", "比较的值"), "string", "0",
                  QT_TRANSLATE_NOOP("Instructions", "与变量比较的值，如 3 或「成功」；判断软件时填进程名，如 notepad.exe 或 微信"))
        };
        m.outPorts = { "真", "假" };   // 端口标签是流程文件格式的一部分（连线匹配用），不翻译
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string left = ctx.pStr(params, "left");
        std::string op   = ctx.pStr(params, "op", "等于");
        std::string right = ctx.pStr(params, "right");
        std::string preset = ctx.pStr(params, "preset", "自定义");

        bool r = false;
        // 进程检测类预设：直接用「比较的值」填进程名
        if (preset == "软件已打开") {
            r = isProcessRunning(right);
            ctx.info(QCoreApplication::translate("Instructions", "软件已打开：%1 → %2")
                         .arg(QString::fromStdString(right)).arg(r ? "真" : "假").toStdString());
            return r ? "真" : "假";
        } else if (preset == "软件未打开") {
            r = !isProcessRunning(right);
            ctx.info(QCoreApplication::translate("Instructions", "软件未打开：%1 → %2")
                         .arg(QString::fromStdString(right)).arg(r ? "真" : "假").toStdString());
            return r ? "真" : "假";
        }

        // 快捷预设：自动覆盖变量/条件（「OCR 识别到文字」的右边值仍用用户填的「比较的值」）
        if (preset == "找图成功")          { left = "${match.found}";     op = "=="; right = "true"; }
        else if (preset == "找图失败")     { left = "${match.found}";     op = "!="; right = "true"; }
        else if (preset == "AI 识别到目标")   { left = "${aiResult.found}"; op = "=="; right = "true"; }
        else if (preset == "AI 未识别到目标") { left = "${aiResult.found}"; op = "!="; right = "true"; }
        else if (preset == "OCR 识别到文字")  { left = "${ocrText}";        op = "contains"; /* right 用用户填的值 */ }

        Condition c;
        c.left  = left;
        c.op    = normalizeOp(op);
        c.right = right;
        r = evaluateCondition(c, ctx.vars);
        ctx.info((preset != "自定义" ? preset : left + " " + op + " " + right)
                 + " → " + (r ? "真" : "假"));
        return r ? "真" : "假";
    }
};

struct LoopInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "loop"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "循环");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "重复执行内嵌步骤 count 次");
        m.params = {
            Param("count",    QT_TRANSLATE_NOOP("Instructions", "循环次数"), "int", "3",
                  QT_TRANSLATE_NOOP("Instructions", "例如 3 次")),
            Param("indexVar", QT_TRANSLATE_NOOP("Instructions", "索引变量名"), "string", "i",
                  QT_TRANSLATE_NOOP("Instructions", "每次循环把第 n 次(从 1 开始)写入该变量，可留空")),
            Param("body",     QT_TRANSLATE_NOOP("Instructions", "循环体(JSON)"), "textarea",
                  "{\n  \"nodes\": [\n    { \"id\": \"b1\", \"instr\": \"log\", \"params\": { \"text\": \"第 ${i} 次循环\" } }\n  ],\n  \"edges\": []\n}",
                  QT_TRANSLATE_NOOP("Instructions", "内嵌步骤的 JSON 描述，节点按顺序执行"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int count = ctx.pInt(params, "count", 1);
        std::string indexVar = ctx.pStr(params, "indexVar");
        json body = params.value("body", json::object());
        if (body.is_string()) {
            try { body = json::parse(body.get<std::string>()); }
            catch (...) { ctx.error = QCoreApplication::translate("Instructions", "循环体 JSON 解析失败").toStdString(); return ""; }
        }
        for (int i = 0; i < count; ++i) {
            if (ctx.stopFlag && ctx.stopFlag->load()) break;
            if (!indexVar.empty()) {
                ctx.vars.set(indexVar, Variable::makeNumber(i + 1));
                if (ctx.notifyVar) ctx.notifyVar(indexVar);
            }
            ctx.info(QCoreApplication::translate("Instructions", "循环第 %1 / %2 次")
                         .arg(i + 1).arg(count).toStdString());
            if (ctx.runSubFlow && !ctx.runSubFlow(body)) {
                if (ctx.error.empty()) ctx.error = QCoreApplication::translate("Instructions", "循环体执行失败").toStdString();
                return "";
            }
        }
        return "next";
    }
};

struct JumpInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "jump"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "跳转");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "跳转到指定步骤节点");
        m.params = { Param("target", QT_TRANSLATE_NOOP("Instructions", "跳转目标"), "node", "",
                           QT_TRANSLATE_NOOP("Instructions", "选择要跳转到的节点")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string target = ctx.pStr(params, "target");
        if (target.empty()) { ctx.error = QCoreApplication::translate("Instructions", "未指定跳转目标").toStdString(); return ""; }
        ctx.jumpTarget = target;
        ctx.info(QCoreApplication::translate("Instructions", "跳转到节点 %1")
                     .arg(QString::fromStdString(target)).toStdString());
        return "next";
    }
};

struct SubflowInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "subflow"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "子流程");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "运行另一个流程文件或内嵌子流程");
        m.params = {
            Param("flowPath", QT_TRANSLATE_NOOP("Instructions", "流程文件路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "留空则使用内嵌流程")),
            Param("body",     QT_TRANSLATE_NOOP("Instructions", "内嵌流程(JSON)"), "textarea",
                  "{\n  \"nodes\": [\n    { \"id\": \"s1\", \"instr\": \"log\", \"params\": { \"text\": \"子流程执行\" } }\n  ],\n  \"edges\": []\n}",
                  QT_TRANSLATE_NOOP("Instructions", "当流程文件路径为空时使用"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        json body;
        std::string flowPath = ctx.pStr(params, "flowPath");
        if (!flowPath.empty()) {
            FlowModel sub;
            std::string err;
            if (!sub.loadFromFile(flowPath, err)) {
                ctx.error = QCoreApplication::translate("Instructions", "加载子流程失败: %1")
                                .arg(QString::fromStdString(err)).toStdString();
                return "";
            }
            body = sub.toJson();
        } else {
            body = params.value("body", json::object());
            if (body.is_string()) {
                try { body = json::parse(body.get<std::string>()); }
                catch (...) { ctx.error = QCoreApplication::translate("Instructions", "子流程 JSON 解析失败").toStdString(); return ""; }
            }
        }
        ctx.info(QCoreApplication::translate("Instructions", "进入子流程").toStdString());
        if (ctx.runSubFlow && !ctx.runSubFlow(body)) {
            if (ctx.error.empty()) ctx.error = QCoreApplication::translate("Instructions", "子流程执行失败").toStdString();
            return "";
        }
        return "next";
    }
};

struct LogInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "log"; m.category = Category::Flow; m.name = QT_TRANSLATE_NOOP("Instructions", "输出日志");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "在运行日志中输出一条消息");
        m.params = { Param("text", QT_TRANSLATE_NOOP("Instructions", "日志内容"), "string", "",
                           QT_TRANSLATE_NOOP("Instructions", "支持 ${变量} 引用")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        ctx.info(ctx.pStr(params, "text"));
        return "next";
    }
};

void registerFlowInstructions() {
    registerInstruction(std::make_unique<StartInstr>());
    registerInstruction(std::make_unique<EndInstr>());
    registerInstruction(std::make_unique<DelayInstr>());
    registerInstruction(std::make_unique<IfInstr>());
    registerInstruction(std::make_unique<LoopInstr>());
    registerInstruction(std::make_unique<JumpInstr>());
    registerInstruction(std::make_unique<SubflowInstr>());
    registerInstruction(std::make_unique<LogInstr>());
}

} // namespace autoflow
