#include "IInstruction.h"
#include "InstructionRegistry.h"
#include "../infra/InputSimulator.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

namespace autoflow {

// ============================ 键鼠类指令 ============================

struct ClickInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "click"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "鼠标点击");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "在屏幕坐标处点击（绝对像素坐标）");
        m.params = {
            Param("x", QT_TRANSLATE_NOOP("Instructions", "X 坐标"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "屏幕水平像素，左上角为 0")),
            Param("y", QT_TRANSLATE_NOOP("Instructions", "Y 坐标"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "屏幕垂直像素")),
            Param("button", QT_TRANSLATE_NOOP("Instructions", "按键"), "select", "left",
                  QT_TRANSLATE_NOOP("Instructions", "左键/右键/中键/双击")).withOptions({ "left", "right", "middle", "double" })
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int x = ctx.pInt(params, "x");
        int y = ctx.pInt(params, "y");
        std::string button = ctx.pStr(params, "button", "left");
        InputSimulator::mouseClick(x, y, button);
        ctx.info(QCoreApplication::translate("Instructions", "点击 (%1, %2) %3")
                     .arg(x).arg(y).arg(QString::fromStdString(button)).toStdString());
        return "next";
    }
};

struct MoveInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "move"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "鼠标移动");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "移动鼠标到指定坐标，可选平滑移动");
        m.params = {
            Param("x", QT_TRANSLATE_NOOP("Instructions", "X 坐标"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "屏幕水平像素，左上角为 0")),
            Param("y", QT_TRANSLATE_NOOP("Instructions", "Y 坐标"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "屏幕垂直像素")),
            Param("duration", QT_TRANSLATE_NOOP("Instructions", "移动时长(毫秒)"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "0=瞬移；>0 平滑移动，如 500 表示用 0.5 秒移过去"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int x = ctx.pInt(params, "x");
        int y = ctx.pInt(params, "y");
        int duration = ctx.pInt(params, "duration", 0);
        if (duration > 0) InputSimulator::mouseMoveSmooth(x, y, duration);
        else InputSimulator::mouseMove(x, y);
        ctx.info(QCoreApplication::translate("Instructions", "鼠标移动到 (%1, %2)%3")
                     .arg(x).arg(y)
                     .arg(duration > 0 ? QString("（平滑 %1ms）").arg(duration) : QString())
                     .toStdString());
        return "next";
    }
};

struct DragInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "drag"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "鼠标拖拽");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "按住按键从起点拖到终点");
        m.params = {
            Param("x1", QT_TRANSLATE_NOOP("Instructions", "起点 X"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "拖拽起始水平坐标")),
            Param("y1", QT_TRANSLATE_NOOP("Instructions", "起点 Y"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "拖拽起始垂直坐标")),
            Param("x2", QT_TRANSLATE_NOOP("Instructions", "终点 X"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "拖拽终点水平坐标")),
            Param("y2", QT_TRANSLATE_NOOP("Instructions", "终点 Y"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "拖拽终点垂直坐标")),
            Param("button", QT_TRANSLATE_NOOP("Instructions", "按键"), "select", "left",
                  QT_TRANSLATE_NOOP("Instructions", "左键/右键")).withOptions({ "left", "right" }),
            Param("duration", QT_TRANSLATE_NOOP("Instructions", "拖动时长(毫秒)"), "int", "300",
                  QT_TRANSLATE_NOOP("Instructions", "拖动过程的平滑移动时长"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int x1 = ctx.pInt(params, "x1"), y1 = ctx.pInt(params, "y1");
        int x2 = ctx.pInt(params, "x2"), y2 = ctx.pInt(params, "y2");
        std::string button = ctx.pStr(params, "button", "left");
        int duration = ctx.pInt(params, "duration", 300);
        InputSimulator::mouseDrag(x1, y1, x2, y2, button, duration);
        ctx.info(QCoreApplication::translate("Instructions", "拖拽 (%1, %2) → (%3, %4) %5")
                     .arg(x1).arg(y1).arg(x2).arg(y2)
                     .arg(QString::fromStdString(button)).toStdString());
        return "next";
    }
};

struct GetMousePosInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "getmousepos"; m.category = Category::Input;
        m.name = QT_TRANSLATE_NOOP("Instructions", "获取鼠标位置");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "读取当前鼠标坐标到变量");
        m.params = {
            Param("xVar", QT_TRANSLATE_NOOP("Instructions", "X 变量"), "string", "mouseX",
                  QT_TRANSLATE_NOOP("Instructions", "保存鼠标 X 坐标的变量名")),
            Param("yVar", QT_TRANSLATE_NOOP("Instructions", "Y 变量"), "string", "mouseY",
                  QT_TRANSLATE_NOOP("Instructions", "保存鼠标 Y 坐标的变量名"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int x = 0, y = 0;
        if (!InputSimulator::getMousePos(x, y)) {
            ctx.error = QCoreApplication::translate("Instructions", "获取鼠标位置失败").toStdString();
            return "";
        }
        std::string xVar = ctx.pStr(params, "xVar", "mouseX");
        std::string yVar = ctx.pStr(params, "yVar", "mouseY");
        ctx.vars.set(xVar, Variable::makeNumber(x));
        ctx.vars.set(yVar, Variable::makeNumber(y));
        if (ctx.notifyVar) { ctx.notifyVar(xVar); ctx.notifyVar(yVar); }
        ctx.info(QCoreApplication::translate("Instructions", "鼠标位置 (%1, %2) 已保存到 %3 / %4")
                     .arg(x).arg(y)
                     .arg(QString::fromStdString(xVar)).arg(QString::fromStdString(yVar)).toStdString());
        return "next";
    }
};

struct KeyboardInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "keyboard"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "键盘输入");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "键入文本（支持中文）");
        m.params = { Param("text", QT_TRANSLATE_NOOP("Instructions", "输入内容"), "string", "",
                           QT_TRANSLATE_NOOP("Instructions", "支持 ${变量} 引用")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string text = ctx.pStr(params, "text");
        InputSimulator::keyType(text);
        ctx.info(QCoreApplication::translate("Instructions", "键入文本: %1")
                     .arg(QString::fromStdString(text)).toStdString());
        return "next";
    }
};

struct HotkeyInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "hotkey"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "快捷键");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "按下组合键，如 ctrl+c");
        m.params = { Param("keys", QT_TRANSLATE_NOOP("Instructions", "组合键"), "string", "ctrl+c",
                           QT_TRANSLATE_NOOP("Instructions", "例如 ctrl+shift+s、alt+tab")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string keys = ctx.pStr(params, "keys", "ctrl+c");
        if (!InputSimulator::keyCombo(keys)) {
            ctx.error = QCoreApplication::translate("Instructions", "无法识别的组合键: %1")
                            .arg(QString::fromStdString(keys)).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "按下组合键 %1")
                     .arg(QString::fromStdString(keys)).toStdString());
        return "next";
    }
};

struct ScrollInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "scroll"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "滚轮滚动");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "滚动鼠标滚轮");
        m.params = { Param("delta", QT_TRANSLATE_NOOP("Instructions", "滚动量"), "int", "120",
                           QT_TRANSLATE_NOOP("Instructions", "正数向上，负数向下（120 ≈ 一格）")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        int delta = ctx.pInt(params, "delta", 120);
        InputSimulator::mouseWheel(delta);
        ctx.info(QCoreApplication::translate("Instructions", "滚轮滚动 %1").arg(delta).toStdString());
        return "next";
    }
};

struct OpenUrlInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "openurl"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "打开网页");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "用默认浏览器打开网址");
        m.params = { Param("url", QT_TRANSLATE_NOOP("Instructions", "网址"), "string",
                           "https://www.baidu.com", QT_TRANSLATE_NOOP("Instructions", "完整网址")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string url = ctx.pStr(params, "url");
        if (url.empty()) { ctx.error = QCoreApplication::translate("Instructions", "网址为空").toStdString(); return ""; }
        HINSTANCE r = ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        if ((INT_PTR)r <= 32) {
            ctx.error = QCoreApplication::translate("Instructions", "打开网页失败: %1")
                            .arg(QString::fromStdString(url)).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "已打开网页 %1")
                     .arg(QString::fromStdString(url)).toStdString());
        return "next";
    }
};

struct ClipboardInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "clipboard"; m.category = Category::Input; m.name = QT_TRANSLATE_NOOP("Instructions", "获取剪贴板");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "读取剪贴板文本到变量");
        m.params = { Param("var", QT_TRANSLATE_NOOP("Instructions", "保存到变量"), "string",
                           "clipboard", QT_TRANSLATE_NOOP("Instructions", "变量名")) };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string name = ctx.pStr(params, "var", "clipboard");
        std::string text = InputSimulator::getClipboardText();
        ctx.vars.set(name, Variable::makeString(text));
        if (ctx.notifyVar) ctx.notifyVar(name);
        ctx.info(QCoreApplication::translate("Instructions", "剪贴板内容已保存到变量 %1")
                     .arg(QString::fromStdString(name)).toStdString());
        return "next";
    }
};

void registerInputInstructions() {
    registerInstruction(std::make_unique<ClickInstr>());
    registerInstruction(std::make_unique<MoveInstr>());
    registerInstruction(std::make_unique<DragInstr>());
    registerInstruction(std::make_unique<GetMousePosInstr>());
    registerInstruction(std::make_unique<KeyboardInstr>());
    registerInstruction(std::make_unique<HotkeyInstr>());
    registerInstruction(std::make_unique<ScrollInstr>());
    registerInstruction(std::make_unique<OpenUrlInstr>());
    registerInstruction(std::make_unique<ClipboardInstr>());
}

} // namespace autoflow
