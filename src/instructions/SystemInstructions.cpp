// 窗口 / 文件 / 系统类指令（Win32 + Qt）
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "IInstruction.h"
#include "InstructionRegistry.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <thread>
#include <chrono>

namespace autoflow {

// ============================ 窗口类指令 ============================

// 按标题查找顶层窗口；找不到返回 nullptr
static HWND findWindowByTitle(const std::string& title) {
    return FindWindowW(nullptr, QString::fromUtf8(title.c_str()).toStdWString().c_str());
}

struct WinActivateInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "winactivate"; m.category = Category::Window; m.name = QT_TRANSLATE_NOOP("Instructions", "激活窗口");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "按标题查找窗口并把它带到前台");
        m.params = {
            Param("title", QT_TRANSLATE_NOOP("Instructions", "窗口标题"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}，例如 记事本"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string title = ctx.pStr(params, "title");
        HWND hwnd = findWindowByTitle(title);
        if (!hwnd) {
            ctx.error = QCoreApplication::translate("Instructions", "未找到窗口: %1")
                            .arg(QString::fromStdString(title)).toStdString();
            return "";
        }
        // 恢复最小化的窗口
        if (IsIconic(hwnd)) ShowWindow(hwnd, SW_RESTORE);
        // 模拟 Alt 键按下再释放：绕过 Windows 前台窗口限制（最可靠的方法）
        keybd_event(VK_MENU, 0, 0, 0);
        keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
        // AttachThreadInput 进一步确保前台切换生效
        DWORD foreThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
        DWORD targetThread = GetWindowThreadProcessId(hwnd, nullptr);
        if (foreThread != targetThread) {
            AttachThreadInput(foreThread, targetThread, TRUE);
            SetForegroundWindow(hwnd);
            BringWindowToTop(hwnd);
            ShowWindow(hwnd, SW_SHOW);
            AttachThreadInput(foreThread, targetThread, FALSE);
        } else {
            SetForegroundWindow(hwnd);
            BringWindowToTop(hwnd);
        }
        ctx.info(QCoreApplication::translate("Instructions", "已激活窗口: %1")
                     .arg(QString::fromStdString(title)).toStdString());
        return "next";
    }
};

struct WinWaitInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "winwait"; m.category = Category::Window; m.name = QT_TRANSLATE_NOOP("Instructions", "等待窗口");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "等待指定标题的窗口出现");
        m.params = {
            Param("title",   QT_TRANSLATE_NOOP("Instructions", "窗口标题"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("timeout", QT_TRANSLATE_NOOP("Instructions", "超时(毫秒)"), "int", "10000",
                  QT_TRANSLATE_NOOP("Instructions", "例如 10000 = 10 秒"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string title = ctx.pStr(params, "title");
        int timeout = ctx.pInt(params, "timeout", 10000);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
        while (std::chrono::steady_clock::now() < deadline) {
            if (findWindowByTitle(title)) {
                ctx.info(QCoreApplication::translate("Instructions", "窗口已出现: %1")
                             .arg(QString::fromStdString(title)).toStdString());
                return "next";
            }
            if (ctx.stopFlag && ctx.stopFlag->load()) { ctx.error = QCoreApplication::translate("Instructions", "已停止").toStdString(); return ""; }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        ctx.error = QCoreApplication::translate("Instructions", "等待窗口超时: %1")
                        .arg(QString::fromStdString(title)).toStdString();
        return "";
    }
};

struct WinStateInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "winstate"; m.category = Category::Window; m.name = QT_TRANSLATE_NOOP("Instructions", "设置窗口状态");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "最大化 / 最小化 / 还原窗口");
        m.params = {
            Param("title", QT_TRANSLATE_NOOP("Instructions", "窗口标题"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            // state 选项值会存入流程 JSON 并参与运行时比较，属于数据，不翻译
            Param("state", QT_TRANSLATE_NOOP("Instructions", "状态"), "select", "最大化", "").withOptions({ "最大化", "最小化", "还原" })
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string title = ctx.pStr(params, "title");
        std::string state = ctx.pStr(params, "state", "最大化");
        HWND hwnd = findWindowByTitle(title);
        if (!hwnd) {
            ctx.error = QCoreApplication::translate("Instructions", "未找到窗口: %1")
                            .arg(QString::fromStdString(title)).toStdString();
            return "";
        }
        int cmd = (state == "最大化") ? SW_MAXIMIZE : (state == "最小化") ? SW_MINIMIZE : SW_RESTORE;
        ShowWindow(hwnd, cmd);
        ctx.info(QCoreApplication::translate("Instructions", "窗口 %1 状态已设置为 %2")
                     .arg(QString::fromStdString(title)).arg(QString::fromStdString(state)).toStdString());
        return "next";
    }
};

struct WinCloseInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "winclose"; m.category = Category::Window; m.name = QT_TRANSLATE_NOOP("Instructions", "关闭窗口");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "向窗口发送关闭消息");
        m.params = {
            Param("title", QT_TRANSLATE_NOOP("Instructions", "窗口标题"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string title = ctx.pStr(params, "title");
        HWND hwnd = findWindowByTitle(title);
        if (!hwnd) {
            ctx.error = QCoreApplication::translate("Instructions", "未找到窗口: %1")
                            .arg(QString::fromStdString(title)).toStdString();
            return "";
        }
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
        ctx.info(QCoreApplication::translate("Instructions", "已发送关闭消息到窗口: %1")
                     .arg(QString::fromStdString(title)).toStdString());
        return "next";
    }
};

struct WinGetInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "winget"; m.category = Category::Window; m.name = QT_TRANSLATE_NOOP("Instructions", "获取窗口信息");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "获取窗口的标题 / 位置 / 大小");
        m.params = {
            Param("title", QT_TRANSLATE_NOOP("Instructions", "窗口标题"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            // prop 选项值会存入流程 JSON 并参与运行时比较，属于数据，不翻译
            Param("prop", QT_TRANSLATE_NOOP("Instructions", "信息类型"), "select", "标题", "").withOptions({ "标题", "位置", "大小" }),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果变量"), "string", "winInfo",
                  QT_TRANSLATE_NOOP("Instructions", "标题存字符串；位置存 {left,top}；大小存 {width,height}"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string title = ctx.pStr(params, "title");
        std::string prop = ctx.pStr(params, "prop", "标题");
        std::string saveVar = ctx.pStr(params, "saveVar", "winInfo");
        HWND hwnd = findWindowByTitle(title);
        if (!hwnd) {
            ctx.error = QCoreApplication::translate("Instructions", "未找到窗口: %1")
                            .arg(QString::fromStdString(title)).toStdString();
            return "";
        }
        if (prop == "标题") {
            wchar_t buf[512] = {};
            GetWindowTextW(hwnd, buf, 512);
            ctx.vars.set(saveVar, Variable::makeString(QString::fromWCharArray(buf).toStdString()));
        } else {
            RECT rc{};
            if (!GetWindowRect(hwnd, &rc)) {
                ctx.error = QCoreApplication::translate("Instructions", "无法获取窗口位置: %1")
                                .arg(QString::fromStdString(title)).toStdString();
                return "";
            }
            Variable obj = Variable::makeObject();
            if (prop == "位置") {
                obj.setChild("left", Variable::makeNumber(rc.left));
                obj.setChild("top", Variable::makeNumber(rc.top));
            } else { // 大小
                obj.setChild("width", Variable::makeNumber(rc.right - rc.left));
                obj.setChild("height", Variable::makeNumber(rc.bottom - rc.top));
            }
            ctx.vars.set(saveVar, obj);
        }
        if (ctx.notifyVar) ctx.notifyVar(saveVar);
        ctx.info(QCoreApplication::translate("Instructions", "窗口 %1 的%2已保存到 %3")
                     .arg(QString::fromStdString(title)).arg(QString::fromStdString(prop))
                     .arg(QString::fromStdString(saveVar)).toStdString());
        return "next";
    }
};

// ============================ 文件类指令 ============================

struct FileCopyInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "filecopy"; m.category = Category::File; m.name = QT_TRANSLATE_NOOP("Instructions", "复制文件");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "把文件从源路径复制到目标路径");
        m.params = {
            Param("src", QT_TRANSLATE_NOOP("Instructions", "源路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("dst", QT_TRANSLATE_NOOP("Instructions", "目标路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "目标已存在时会先删除再复制"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string src = ctx.pStr(params, "src");
        std::string dst = ctx.pStr(params, "dst");
        // QFile::copy 在目标已存在时失败，先尝试删除目标
        if (QFile::exists(QString::fromStdString(dst))) QFile::remove(QString::fromStdString(dst));
        if (!QFile::copy(QString::fromStdString(src), QString::fromStdString(dst))) {
            ctx.error = QCoreApplication::translate("Instructions", "复制文件失败: %1 → %2")
                            .arg(QString::fromStdString(src)).arg(QString::fromStdString(dst)).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "已复制 %1 → %2")
                     .arg(QString::fromStdString(src)).arg(QString::fromStdString(dst)).toStdString());
        return "next";
    }
};

struct FileMoveInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "filemove"; m.category = Category::File; m.name = QT_TRANSLATE_NOOP("Instructions", "移动文件");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "把文件从源路径移动到目标路径");
        m.params = {
            Param("src", QT_TRANSLATE_NOOP("Instructions", "源路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("dst", QT_TRANSLATE_NOOP("Instructions", "目标路径"), "string", "", "")
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string src = ctx.pStr(params, "src");
        std::string dst = ctx.pStr(params, "dst");
        QString qsrc = QString::fromStdString(src);
        QString qdst = QString::fromStdString(dst);
        if (QFile::exists(qdst)) QFile::remove(qdst);
        // 同盘 rename；跨盘 rename 失败时退化为 copy + remove
        if (!QFile::rename(qsrc, qdst)) {
            if (!QFile::copy(qsrc, qdst) || !QFile::remove(qsrc)) {
                ctx.error = QCoreApplication::translate("Instructions", "移动文件失败: %1 → %2")
                                .arg(qsrc).arg(qdst).toStdString();
                return "";
            }
        }
        ctx.info(QCoreApplication::translate("Instructions", "已移动 %1 → %2").arg(qsrc).arg(qdst).toStdString());
        return "next";
    }
};

struct FileDeleteInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "filedelete"; m.category = Category::File; m.name = QT_TRANSLATE_NOOP("Instructions", "删除文件");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "删除指定文件");
        m.params = {
            Param("path", QT_TRANSLATE_NOOP("Instructions", "文件路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string path = ctx.pStr(params, "path");
        if (!QFile::remove(QString::fromStdString(path))) {
            ctx.error = QCoreApplication::translate("Instructions", "删除文件失败: %1")
                            .arg(QString::fromStdString(path)).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "已删除 %1")
                     .arg(QString::fromStdString(path)).toStdString());
        return "next";
    }
};

struct FileReadInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "fileread"; m.category = Category::File; m.name = QT_TRANSLATE_NOOP("Instructions", "读取文本文件");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "读取文本文件内容到变量");
        m.params = {
            Param("path",    QT_TRANSLATE_NOOP("Instructions", "文件路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果变量"), "string", "fileContent",
                  QT_TRANSLATE_NOOP("Instructions", "文件内容按 UTF-8 读取"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string path = ctx.pStr(params, "path");
        std::string saveVar = ctx.pStr(params, "saveVar", "fileContent");
        QFile f(QString::fromStdString(path));
        if (!f.open(QIODevice::ReadOnly)) {
            ctx.error = QCoreApplication::translate("Instructions", "无法读取文件: %1")
                            .arg(QString::fromStdString(path)).toStdString();
            return "";
        }
        QString content = QString::fromUtf8(f.readAll());
        ctx.vars.set(saveVar, Variable::makeString(content.toStdString()));
        if (ctx.notifyVar) ctx.notifyVar(saveVar);
        ctx.info(QCoreApplication::translate("Instructions", "已读取 %1 到变量 %2（%3 字节）")
                     .arg(QString::fromStdString(path)).arg(QString::fromStdString(saveVar))
                     .arg((qulonglong)content.toUtf8().size()).toStdString());
        return "next";
    }
};

struct FileWriteInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "filewrite"; m.category = Category::File; m.name = QT_TRANSLATE_NOOP("Instructions", "写入文本文件");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "把文本写入文件（覆盖或追加）");
        m.params = {
            Param("path",    QT_TRANSLATE_NOOP("Instructions", "文件路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("content", QT_TRANSLATE_NOOP("Instructions", "内容"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            // mode 选项值会存入流程 JSON 并参与运行时比较，属于数据，不翻译
            Param("mode", QT_TRANSLATE_NOOP("Instructions", "写入模式"), "select", "覆盖", "").withOptions({ "覆盖", "追加" })
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string path = ctx.pStr(params, "path");
        std::string content = ctx.pStr(params, "content");
        std::string mode = ctx.pStr(params, "mode", "覆盖");
        QFile f(QString::fromStdString(path));
        QIODevice::OpenMode om = QIODevice::WriteOnly;
        if (mode == "追加") om |= QIODevice::Append;
        if (!f.open(om)) {
            ctx.error = QCoreApplication::translate("Instructions", "无法写入文件: %1")
                            .arg(QString::fromStdString(path)).toStdString();
            return "";
        }
        QByteArray data = QString::fromStdString(content).toUtf8();
        if (f.write(data) != data.size()) {
            ctx.error = QCoreApplication::translate("Instructions", "写入文件失败: %1")
                            .arg(QString::fromStdString(path)).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "已写入 %1（%2 字节，%3）")
                     .arg(QString::fromStdString(path)).arg((qulonglong)data.size())
                     .arg(QString::fromStdString(mode)).toStdString());
        return "next";
    }
};

struct FileExistsInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "fileexists"; m.category = Category::File; m.name = QT_TRANSLATE_NOOP("Instructions", "文件是否存在");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "判断文件是否存在，结果存布尔变量");
        m.params = {
            Param("path",    QT_TRANSLATE_NOOP("Instructions", "文件路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "结果变量"), "string", "exists", "")
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string path = ctx.pStr(params, "path");
        std::string saveVar = ctx.pStr(params, "saveVar", "exists");
        bool exists = QFile::exists(QString::fromStdString(path));
        ctx.vars.set(saveVar, Variable::makeBool(exists));
        if (ctx.notifyVar) ctx.notifyVar(saveVar);
        ctx.info(QCoreApplication::translate("Instructions", "%1 %2，结果已保存到 %3")
                     .arg(QString::fromStdString(path))
                     .arg(exists ? QCoreApplication::translate("Instructions", "存在")
                                 : QCoreApplication::translate("Instructions", "不存在"))
                     .arg(QString::fromStdString(saveVar)).toStdString());
        return "next";
    }
};

struct MkDirInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "mkdir"; m.category = Category::File; m.name = QT_TRANSLATE_NOOP("Instructions", "创建目录");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "创建目录（含多级父目录）");
        m.params = {
            Param("path", QT_TRANSLATE_NOOP("Instructions", "目录路径"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string path = ctx.pStr(params, "path");
        if (!QDir().mkpath(QString::fromStdString(path))) {
            ctx.error = QCoreApplication::translate("Instructions", "创建目录失败: %1")
                            .arg(QString::fromStdString(path)).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "目录已创建: %1")
                     .arg(QString::fromStdString(path)).toStdString());
        return "next";
    }
};

// ============================ 系统类指令 ============================

struct RunInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "run"; m.category = Category::System; m.name = QT_TRANSLATE_NOOP("Instructions", "启动程序");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "启动外部程序（不等待其退出）");
        m.params = {
            Param("program", QT_TRANSLATE_NOOP("Instructions", "程序路径"), "string", "notepad.exe",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("args",    QT_TRANSLATE_NOOP("Instructions", "命令行参数"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "以空格分隔，可留空")).opt(),
            Param("workdir", QT_TRANSLATE_NOOP("Instructions", "工作目录"), "string", "",
                  QT_TRANSLATE_NOOP("Instructions", "可留空")).opt()
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string program = ctx.pStr(params, "program");
        std::string args = ctx.pStr(params, "args");
        std::string workdir = ctx.pStr(params, "workdir");
        if (program.empty()) {
            ctx.error = QCoreApplication::translate("Instructions", "程序路径为空").toStdString();
            return "";
        }
        QString qprogram = QString::fromStdString(program);
        QStringList qargs = args.empty()
            ? QStringList()
            : QString::fromStdString(args).split(' ', Qt::SkipEmptyParts);
        QString qworkdir = workdir.empty() ? QString() : QString::fromStdString(workdir);
        if (!QProcess::startDetached(qprogram, qargs, qworkdir)) {
            ctx.error = QCoreApplication::translate("Instructions", "启动程序失败: %1")
                            .arg(qprogram).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "已启动程序: %1").arg(qprogram).toStdString());
        return "next";
    }
};

struct KillInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "kill"; m.category = Category::System; m.name = QT_TRANSLATE_NOOP("Instructions", "结束进程");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "按进程名或 PID 强制结束进程");
        m.params = {
            Param("name", QT_TRANSLATE_NOOP("Instructions", "进程名"), "string", "notepad",
                  QT_TRANSLATE_NOOP("Instructions", "例如 notepad，可留空改用 PID")).opt(),
            Param("pid",  QT_TRANSLATE_NOOP("Instructions", "进程ID"), "int", "0",
                  QT_TRANSLATE_NOOP("Instructions", "大于 0 时优先使用")).opt()
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string name = ctx.pStr(params, "name");
        int pid = ctx.pInt(params, "pid", 0);
        QStringList args;
        QString target;
        if (pid > 0) {
            args = { "/F", "/T", "/PID", QString::number(pid) };
            target = QString::number(pid);
        } else {
            if (name.empty()) {
                ctx.error = QCoreApplication::translate("Instructions", "进程名和 PID 不能同时为空").toStdString();
                return "";
            }
            QString qname = QString::fromStdString(name);
            if (!qname.endsWith(".exe", Qt::CaseInsensitive)) qname += ".exe";
            args = { "/F", "/T", "/IM", qname };
            target = qname;
        }
        int rc = QProcess::execute("taskkill", args);
        if (rc != 0) {
            ctx.error = QCoreApplication::translate("Instructions", "结束进程失败: %1（返回码 %2）")
                            .arg(target).arg(rc).toStdString();
            return "";
        }
        ctx.info(QCoreApplication::translate("Instructions", "已结束进程: %1").arg(target).toStdString());
        return "next";
    }
};

struct CmdInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "cmd"; m.category = Category::System; m.name = QT_TRANSLATE_NOOP("Instructions", "运行命令");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "运行 cmd 命令并把输出保存到变量");
        m.params = {
            Param("command", QT_TRANSLATE_NOOP("Instructions", "命令"), "string", "dir",
                  QT_TRANSLATE_NOOP("Instructions", "支持 ${变量}")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "输出变量"), "string", "cmdOut",
                  QT_TRANSLATE_NOOP("Instructions", "标准输出 + 标准错误"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string command = ctx.pStr(params, "command");
        std::string saveVar = ctx.pStr(params, "saveVar", "cmdOut");
        if (command.empty()) {
            ctx.error = QCoreApplication::translate("Instructions", "命令为空").toStdString();
            return "";
        }
        QProcess proc;
        proc.start("cmd", { "/c", QString::fromStdString(command) });
        if (!proc.waitForStarted(10000) || !proc.waitForFinished(60000)) {
            ctx.error = QCoreApplication::translate("Instructions", "命令执行失败: %1")
                            .arg(QString::fromStdString(command)).toStdString();
            return "";
        }
        QString out = QString::fromLocal8Bit(proc.readAllStandardOutput())
                    + QString::fromLocal8Bit(proc.readAllStandardError());
        ctx.vars.set(saveVar, Variable::makeString(out.toStdString()));
        if (ctx.notifyVar) ctx.notifyVar(saveVar);
        ctx.info(QCoreApplication::translate("Instructions", "命令已执行，输出已保存到 %1（%2 字节）")
                     .arg(QString::fromStdString(saveVar)).arg((qulonglong)out.toUtf8().size()).toStdString());
        return "next";
    }
};

struct ScriptInstr : IInstruction {
    Meta meta() const override {
        Meta m;
        m.id = "script"; m.category = Category::System; m.name = QT_TRANSLATE_NOOP("Instructions", "运行脚本");
        m.desc = QT_TRANSLATE_NOOP("Instructions", "运行 PowerShell 或 Python 脚本并把输出保存到变量");
        m.params = {
            // type 选项值会存入流程 JSON 并参与运行时比较，属于数据，不翻译
            Param("type", QT_TRANSLATE_NOOP("Instructions", "脚本类型"), "select", "PowerShell", "").withOptions({ "PowerShell", "Python" }),
            Param("script", QT_TRANSLATE_NOOP("Instructions", "脚本内容或路径"), "textarea", "",
                  QT_TRANSLATE_NOOP("Instructions", "PowerShell 为脚本内容；Python 为脚本路径")),
            Param("saveVar", QT_TRANSLATE_NOOP("Instructions", "输出变量"), "string", "scriptOut",
                  QT_TRANSLATE_NOOP("Instructions", "标准输出 + 标准错误"))
        };
        return m;
    }
    std::string execute(ExecutionContext& ctx, const json& params) override {
        std::string type = ctx.pStr(params, "type", "PowerShell");
        std::string script = ctx.pStr(params, "script");
        std::string saveVar = ctx.pStr(params, "saveVar", "scriptOut");
        if (script.empty()) {
            ctx.error = QCoreApplication::translate("Instructions", "脚本内容为空").toStdString();
            return "";
        }
        QProcess proc;
        if (type == "PowerShell")
            proc.start("powershell", { "-NoProfile", "-Command", QString::fromStdString(script) });
        else
            proc.start("python", { QString::fromStdString(script) });
        if (!proc.waitForStarted(10000) || !proc.waitForFinished(120000)) {
            ctx.error = QCoreApplication::translate("Instructions", "%1 脚本执行失败")
                            .arg(QString::fromStdString(type)).toStdString();
            return "";
        }
        QString out = QString::fromUtf8(proc.readAllStandardOutput())
                    + QString::fromUtf8(proc.readAllStandardError());
        ctx.vars.set(saveVar, Variable::makeString(out.toStdString()));
        if (ctx.notifyVar) ctx.notifyVar(saveVar);
        ctx.info(QCoreApplication::translate("Instructions", "%1 脚本已执行，输出已保存到 %2（%3 字节）")
                     .arg(QString::fromStdString(type)).arg(QString::fromStdString(saveVar))
                     .arg((qulonglong)out.toUtf8().size()).toStdString());
        return "next";
    }
};

void registerSystemInstructions() {
    // 窗口
    registerInstruction(std::make_unique<WinActivateInstr>());
    registerInstruction(std::make_unique<WinWaitInstr>());
    registerInstruction(std::make_unique<WinStateInstr>());
    registerInstruction(std::make_unique<WinCloseInstr>());
    registerInstruction(std::make_unique<WinGetInstr>());
    // 文件
    registerInstruction(std::make_unique<FileCopyInstr>());
    registerInstruction(std::make_unique<FileMoveInstr>());
    registerInstruction(std::make_unique<FileDeleteInstr>());
    registerInstruction(std::make_unique<FileReadInstr>());
    registerInstruction(std::make_unique<FileWriteInstr>());
    registerInstruction(std::make_unique<FileExistsInstr>());
    registerInstruction(std::make_unique<MkDirInstr>());
    // 系统
    registerInstruction(std::make_unique<RunInstr>());
    registerInstruction(std::make_unique<KillInstr>());
    registerInstruction(std::make_unique<CmdInstr>());
    registerInstruction(std::make_unique<ScriptInstr>());
}

} // namespace autoflow
