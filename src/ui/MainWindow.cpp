#define NOMINMAX
#include <windows.h>

#include "MainWindow.h"
#include "InstructionPanel.h"
#include "FlowCanvas.h"
#include "ParamPanel.h"
#include "LogPanel.h"
#include "VariablePanel.h"
#include "ThemeManager.h"
#include "HotkeyManager.h"
#include "ThemeToggle.h"
#include "TitleBar.h"
#include "SettingsDialog.h"
#include "TemplateDialog.h"
#include "DynamicIsland.h"
#include "RunBorder.h"
#include "../core/Settings.h"
#include "../instructions/InstructionRegistry.h"

#include <QToolBar>
#include <QToolButton>
#include <QStatusBar>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>
#include <QAction>
#include <QMenu>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QVBoxLayout>
#include <QApplication>
#include <QCursor>
#include <QMenu>
#include <QMenuBar>
#include <QStyle>
#include <QKeySequence>
#include <QSettings>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QCheckBox>
#include <QTimer>
#include <QCloseEvent>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStringList>
#include <functional>

namespace autoflow {

// ============================ 预设模板 ============================
// 每个模板 = 名称 + 描述 + 节点列表(指令 id / 坐标 / 参数) + 连线列表
struct PresetTemplate {
    QString name;
    QString desc;
    struct Node { std::string instr; double x, y; json params = json::object(); };
    std::vector<Node> nodes;
    struct Edge { int from, to; std::string label = "next"; };   // from/to 为 nodes 下标
    std::vector<Edge> edges;
};

static const std::vector<PresetTemplate>& presetTemplates() {
    // 模板名/描述用 QT_TRANSLATE_NOOP 标记（运行时恒等），显示处以 MainWindow::tr() 翻译
    static const std::vector<PresetTemplate> tpls = {
        { QT_TRANSLATE_NOOP("autoflow::MainWindow", "找图并点击"),   // start → findimage(找到后点击) → end
          QT_TRANSLATE_NOOP("autoflow::MainWindow", "在屏幕上找到模板图片后自动点击其位置"),
          { { "start",     90, 160 },
            { "findimage", 300, 160, { { "clickIfFound", true } } },
            { "end",       520, 160 } },
          { { 0, 1 }, { 1, 2 } } },
        { QT_TRANSLATE_NOOP("autoflow::MainWindow", "定时点击"),     // start → delay → click → end
          QT_TRANSLATE_NOOP("autoflow::MainWindow", "延时指定时间后在固定坐标点击"),
          { { "start",  90, 160 },
            { "delay",  280, 160, { { "ms", 1000 } } },
            { "click",  470, 160 },
            { "end",    660, 160 } },
          { { 0, 1 }, { 1, 2 }, { 2, 3 } } },
        { QT_TRANSLATE_NOOP("autoflow::MainWindow", "等待画面"),     // start → waitimage → end
          QT_TRANSLATE_NOOP("autoflow::MainWindow", "循环等待目标图片出现，超时则失败"),
          { { "start",     90, 160 },
            { "waitimage", 300, 160 },
            { "end",       520, 160 } },
          { { 0, 1 }, { 1, 2 } } },
        { QT_TRANSLATE_NOOP("autoflow::MainWindow", "循环点击"),     // start → loop(循环体 click) → end
          QT_TRANSLATE_NOOP("autoflow::MainWindow", "重复点击指定次数"),
          { { "start", 90, 160 },
            { "loop",  300, 160,
              { { "count", 3 },
                { "indexVar", "i" },
                { "body", "{\n  \"nodes\": [\n    { \"id\": \"b1\", \"instr\": \"click\", \"params\": {} }\n  ],\n  \"edges\": []\n}" } } },
            { "end",   520, 160 } },
          { { 0, 1 }, { 1, 2 } } },
        { QT_TRANSLATE_NOOP("autoflow::MainWindow", "AI 监控点击"),   // start → loop(ai_vision 识别点击 → delay 1s) → end
          QT_TRANSLATE_NOOP("autoflow::MainWindow", "每秒用 AI 识别目标，识别到后自动点击其位置"),
          { { "start",  90, 160 },
            { "loop",  300, 160,
              { { "count", 3600 },
                { "indexVar", "i" },
                { "body", "{\n  \"nodes\": [\n    { \"id\": \"v1\", \"instr\": \"ai_vision\", \"params\": { \"prompt\": \"画面里有没有你要监控的目标？如果有，返回它的中心坐标\", \"clickIfFound\": true } },\n    { \"id\": \"d1\", \"instr\": \"delay\", \"params\": { \"ms\": 1000 } }\n  ],\n  \"edges\": [ { \"from\": \"v1\", \"to\": \"d1\" } ]\n}" } } },
            { "end",   520, 160 } },
          { { 0, 1 }, { 1, 2 } } },
    };
    return tpls;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();

    // 系统级全局热键：F10 运行 / F12 停止
    m_hotkeys = new HotkeyManager(this);
    m_hotkeys->registerHotkeys();
    connect(m_hotkeys, &HotkeyManager::runRequested, this, [this] { runCurrent(); });
    connect(m_hotkeys, &HotkeyManager::stopRequested, this, [this] { stopRun(); });

    // 主题联动画布
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](bool dark) { m_canvas->setDark(dark); });

    // 启动时恢复主题偏好（m_restoringTheme 防止把 followSystemTheme 误关）
    QSettings settings("AutoFlow", "AutoFlow");
    if (settings.contains("theme/dark")) {
        const bool dark = settings.value("theme/dark").toBool();
        ThemeManager::instance().setDark(dark);
        m_restoringTheme = true;
        m_themeToggle->setDark(dark);
        m_restoringTheme = false;
    } else {
        // 首次启动无历史偏好：默认深色，同步工具栏开关显示
        m_restoringTheme = true;
        m_themeToggle->setDark(ThemeManager::instance().effectiveDark());
        m_restoringTheme = false;
    }

    // 修复：确保画布初始颜色与当前主题一致（首次启动默认深色时画布也要深色）
    m_canvas->setDark(ThemeManager::instance().effectiveDark());

    // 启动时应用开机自启设置（幂等：与注册表 Run 键保持同步）
    applyAutostart(Settings::instance().autostart());

    // 引擎信号
    connect(&m_engine, &ExecutionEngine::nodeStarted, this, &MainWindow::onNodeStarted);
    connect(&m_engine, &ExecutionEngine::nodeFinished, this, &MainWindow::onNodeFinished);
    connect(&m_engine, &ExecutionEngine::logMessage, this, &MainWindow::onLog);
    connect(&m_engine, &ExecutionEngine::runFinished, this, &MainWindow::onRunFinished);
    connect(&m_engine, &ExecutionEngine::variablesSnapshot,
            m_varPanel, &VariablePanel::setSnapshot);
    connect(&m_engine, &ExecutionEngine::runningChanged, this, &MainWindow::onRunningChanged);

    // 灵动岛：引擎信号驱动状态显示
    connect(&m_engine, &ExecutionEngine::runningChanged, this, [this](bool running) {
        if (running && m_island) m_island->setRunning(tr("准备运行…"));
    });
    connect(&m_engine, &ExecutionEngine::nodeStarted, this, [this](const QString& id, const QString& name) {
        if (!m_island) return;
        // 从 FlowModel 取节点参数，构造灵动岛显示
        QList<IslandParam> params;
        int progressMs = 0;   // 时间类模块的预期时长（用于确定性进度条）
        if (FlowNode* n = m_flow.nodeById(id.toStdString())) {
            const IInstruction* ii = InstructionRegistry::instance().get(n->instr);
            if (ii) {
                for (const auto& p : ii->meta().params) {
                    if (p.key == "body" || p.key == "indexVar" || p.type == "textarea")
                        continue;
                    QString val;
                    auto it = n->params.find(p.key);
                    if (it != n->params.end() && it.value().is_string()) {
                        val = QString::fromStdString(it.value().get<std::string>());
                    } else if (it != n->params.end()) {
                        val = QString::fromStdString(it.value().dump());
                    } else {
                        val = QString::fromStdString(p.def);
                    }
                    if (val.length() > 30) val = val.left(30) + "…";
                    params.append({ trInstr(p.label.c_str()), val });
                }
            }
            // 时间类模块：提取实际时长参数用于确定性进度条
            // delay → ms, winwait/waitimage → timeout
            const std::string& instr = n->instr;
            if (instr == "delay") {
                auto it = n->params.find("ms");
                if (it != n->params.end())
                    progressMs = it.value().is_string() ? std::stoi(it.value().get<std::string>()) : (int)it.value().get<double>();
                else
                    progressMs = 1000;
            } else if (instr == "winwait" || instr == "waitimage") {
                auto it = n->params.find("timeout");
                if (it != n->params.end())
                    progressMs = it.value().is_string() ? std::stoi(it.value().get<std::string>()) : (int)it.value().get<double>();
                else
                    progressMs = 10000;
            }
        }
        m_island->setRunning(name, params, progressMs);
    });
    connect(&m_engine, &ExecutionEngine::runFinished, this, [this](bool ok, const QString& summary) {
        if (!m_island) return;
        if (ok) m_island->setResult(summary);
        else m_island->setError(summary);
    });

    // 画布：缩放比例 / 模型变化 / 撤销可用性 / 一次性提示
    connect(m_canvas, &FlowCanvas::zoomChanged, this, &MainWindow::onZoomChanged);
    connect(m_canvas->canvasScene(), &FlowCanvasScene::modelChanged,
            this, &MainWindow::onCanvasModelChanged);
    connect(m_canvas->canvasScene(), &FlowCanvasScene::undoAvailable,
            this, [this](bool can) { if (m_undoAction) m_undoAction->setEnabled(can); });
    connect(m_canvas->canvasScene(), &FlowCanvasScene::redoAvailable,
            this, [this](bool can) { if (m_redoAction) m_redoAction->setEnabled(can); });
    connect(m_canvas->canvasScene(), &FlowCanvasScene::statusMessage,
            this, [this](const QString& msg) { m_status->setText(msg); });

    // 调度器触发 → 运行当前流程
    connect(&m_scheduler, &Scheduler::runTask, this, [this](const QString&, const QString&) {
        runCurrent(QString());
    });

    newFlow();
    updateTitle();

    // 首次运行教程：推迟到窗口显示后再弹出
    QTimer::singleShot(0, this, [this] { showTutorialIfNeeded(); });
}

MainWindow::~MainWindow() {
    if (m_island) { m_island->close(); delete m_island; m_island = nullptr; }
    if (m_runBorder) { m_runBorder->close(); delete m_runBorder; m_runBorder = nullptr; }
}

void MainWindow::buildUi() {
    // 无边框窗口：去掉系统标题栏和边框，用自定义标题栏替代
    setWindowFlag(Qt::FramelessWindowHint, true);

    // 自定义标题栏（放在菜单栏位置，位于工具栏上方）
    m_titleBar = new TitleBar(this);
    setMenuWidget(m_titleBar);
    connect(m_titleBar, &TitleBar::minimizeRequested, this, &QWidget::showMinimized);
    connect(m_titleBar, &TitleBar::maximizeRequested, this, &MainWindow::toggleMaximize);
    connect(m_titleBar, &TitleBar::closeRequested, this, &QWidget::close);

    auto* toolbar = addToolBar("main");
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    // ---- 文件动作 ----
    m_newAction = new QAction(tr("新建"), this);
    m_newAction->setShortcut(QKeySequence::New);
    connect(m_newAction, &QAction::triggered, this, [this] { newFlow(); });

    m_openAction = new QAction(tr("打开"), this);
    m_openAction->setShortcut(QKeySequence::Open);
    connect(m_openAction, &QAction::triggered, this, [this] { openFlow(); });

    m_saveAction = new QAction(tr("保存"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    connect(m_saveAction, &QAction::triggered, this, [this] { saveFlow(); });

    m_saveAsAction = new QAction(tr("另存为"), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAction, &QAction::triggered, this, [this] { saveFlowAs(); });

    m_templateAction = new QAction(tr("模板"), this);
    m_templateAction->setToolTip(tr("模板管理"));
    QMenu* tplMenu = new QMenu(this);
    tplMenu->addAction(tr("模板新建"), this, [this] { newTemplate(); });
    tplMenu->addAction(tr("模板打开"), this, [this] { openTemplate(); });
    tplMenu->addAction(tr("模板保存"), this, [this] { saveTemplate(); });
    tplMenu->addSeparator();
    tplMenu->addAction(tr("浏览模板库…"), this, [this] { openTemplateDialog(); });
    m_templateAction->setMenu(tplMenu);

    QAction* quitAction = new QAction(tr("退出"), this);
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // ---- 编辑动作（快捷键限定在画布内，避免抢占参数面板的文本编辑） ----
    m_undoAction = new QAction(tr("撤销"), this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    m_undoAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_undoAction->setEnabled(false);
    connect(m_undoAction, &QAction::triggered, this, [this] { m_canvas->canvasScene()->undo(); });

    m_redoAction = new QAction(tr("重做"), this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    m_redoAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    m_redoAction->setEnabled(false);
    connect(m_redoAction, &QAction::triggered, this, [this] { m_canvas->canvasScene()->redo(); });

    m_copyAction = new QAction(tr("复制"), this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_copyAction, &QAction::triggered, this, [this] { m_canvas->canvasScene()->copySelection(); });

    m_pasteAction = new QAction(tr("粘贴"), this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_pasteAction, &QAction::triggered, this, [this] { m_canvas->canvasScene()->pasteClipboard(); });

    m_deleteAction = new QAction(tr("删除"), this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_deleteAction, &QAction::triggered, this, [this] { m_canvas->canvasScene()->deleteSelection(); });

    m_selectAllAction = new QAction(tr("全选"), this);
    m_selectAllAction->setShortcut(QKeySequence::SelectAll);
    m_selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(m_selectAllAction, &QAction::triggered, this, [this] { m_canvas->canvasScene()->selectAllNodes(); });

    // ---- 运行动作 ----
    m_runAction = new QAction(tr("运行"), this);
    connect(m_runAction, &QAction::triggered, this, [this] { runCurrent(); });

    m_stopAction = new QAction(tr("停止"), this);
    m_stopAction->setEnabled(false);
    connect(m_stopAction, &QAction::triggered, this, [this] { stopRun(); });

    m_stepAction = new QAction(tr("单步"), this);
    connect(m_stepAction, &QAction::triggered, this, [this] { stepRun(); });

    QAction* runFromAction = new QAction(tr("从此步运行"), this);
    connect(runFromAction, &QAction::triggered, this, [this] { runCurrent(selectedNodeId()); });

    QAction* scheduleAction = new QAction(tr("定时运行"), this);
    connect(scheduleAction, &QAction::triggered, this, [this] { scheduleRun(); });

    m_settingsAction = new QAction(tr("设置"), this);
    connect(m_settingsAction, &QAction::triggered, this, [this] { openSettings(); });

    QAction* tutorialAction = new QAction(tr("新手教程"), this);
    connect(tutorialAction, &QAction::triggered, this, [this] { showTutorialIfNeeded(); });

    // ---- 菜单栏（无边框窗口：作为工具栏首项显示） ----
    auto* menuBar = new QMenuBar(this);
    QMenu* fileMenu = menuBar->addMenu(tr("文件(&F)"));
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    m_recentMenu = new QMenu(tr("最近打开"), fileMenu);
    connect(m_recentMenu, &QMenu::aboutToShow, this, &MainWindow::rebuildRecentMenu);
    fileMenu->addMenu(m_recentMenu);
    fileMenu->addSeparator();
    fileMenu->addAction(m_templateAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    QMenu* editMenu = menuBar->addMenu(tr("编辑(&E)"));
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editMenu->addSeparator();
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(m_deleteAction);
    editMenu->addAction(m_selectAllAction);

    QMenu* runMenu = menuBar->addMenu(tr("运行(&R)"));
    runMenu->addAction(m_runAction);
    runMenu->addAction(m_stopAction);
    runMenu->addSeparator();
    runMenu->addAction(m_stepAction);
    runMenu->addAction(runFromAction);
    runMenu->addAction(scheduleAction);

    QMenu* helpMenu = menuBar->addMenu(tr("帮助(&H)"));
    helpMenu->addAction(tutorialAction);
    helpMenu->addAction(m_settingsAction);

    // ---- 工具栏（高频动作 + 图标） ----
    toolbar->addWidget(menuBar);
    toolbar->addSeparator();
    toolbar->addAction(m_newAction);
    toolbar->addAction(m_openAction);
    toolbar->addAction(m_saveAction);
    toolbar->addAction(m_templateAction);
    if (auto* tplBtn = qobject_cast<QToolButton*>(toolbar->widgetForAction(m_templateAction)))
        tplBtn->setPopupMode(QToolButton::InstantPopup);   // 点击「模板」文字直接弹菜单，无需点箭头
    toolbar->addSeparator();
    toolbar->addAction(m_runAction);
    if (toolbar->widgetForAction(m_runAction))
        toolbar->widgetForAction(m_runAction)->setObjectName("runBtn");
    toolbar->addAction(m_stopAction);
    if (toolbar->widgetForAction(m_stopAction))
        toolbar->widgetForAction(m_stopAction)->setObjectName("stopBtn");

    // “更多 ▾”：低频运行操作折叠
    auto* moreBtn = new QToolButton(toolbar);
    moreBtn->setText(tr("更多"));
    moreBtn->setObjectName("moreBtn");
    moreBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    moreBtn->setPopupMode(QToolButton::InstantPopup);
    moreBtn->setToolTip(tr("更多运行操作"));
    auto* moreMenu = new QMenu(moreBtn);
    moreMenu->addAction(runFromAction);
    moreMenu->addAction(m_stepAction);
    moreMenu->addAction(scheduleAction);
    moreBtn->setMenu(moreMenu);
    toolbar->addWidget(moreBtn);

    toolbar->addSeparator();
    toolbar->addAction(m_undoAction);
    toolbar->addAction(m_redoAction);
    toolbar->addAction(m_deleteAction);
    toolbar->addSeparator();
    QAction* zoomInAction = toolbar->addAction(tr("放大"));
    connect(zoomInAction, &QAction::triggered, this, [this] { m_canvas->zoomIn(); });
    QAction* zoomOutAction = toolbar->addAction(tr("缩小"));
    connect(zoomOutAction, &QAction::triggered, this, [this] { m_canvas->zoomOut(); });
    QAction* zoomFitAction = toolbar->addAction(tr("适配"));
    connect(zoomFitAction, &QAction::triggered, this, [this] { m_canvas->zoomFit(); });
    toolbar->addSeparator();
    toolbar->addAction(m_settingsAction);

    // 昼夜主题开关：手动切换时关闭“跟随系统”，避免被系统主题覆盖
    m_themeToggle = new ThemeToggle(toolbar);
    m_themeToggle->setToolTip(tr("深色 / 浅色主题"));
    toolbar->addWidget(m_themeToggle);
    connect(m_themeToggle, &QAbstractButton::toggled, this, [this](bool dark) {
        if (!m_restoringTheme) Settings::instance().setFollowSystemTheme(false);
        ThemeManager::instance().setDark(dark);
    });

    // 中央布局
    auto* central = new QWidget(this);
    auto* hSplit = new QSplitter(Qt::Horizontal, central);
    hSplit->setChildrenCollapsible(false);
    hSplit->setHandleWidth(6);

    m_instrPanel = new InstructionPanel(hSplit);
    hSplit->addWidget(m_instrPanel);

    // 中列：画布 + 底部日志/变量
    auto* midSplit = new QSplitter(Qt::Vertical, hSplit);
    midSplit->setChildrenCollapsible(false);
    midSplit->setHandleWidth(6);
    m_canvas = new FlowCanvas(&m_flow, midSplit);
    midSplit->addWidget(m_canvas);

    // 编辑动作也注册到画布（WidgetWithChildrenShortcut 仅画布聚焦时生效）
    m_canvas->addAction(m_undoAction);
    m_canvas->addAction(m_redoAction);
    m_canvas->addAction(m_copyAction);
    m_canvas->addAction(m_pasteAction);
    m_canvas->addAction(m_deleteAction);
    m_canvas->addAction(m_selectAllAction);

    auto* bottomTab = new QTabWidget(midSplit);
    m_logPanel = new LogPanel(bottomTab);
    m_varPanel = new VariablePanel(bottomTab);
    bottomTab->addTab(m_logPanel, tr("运行日志"));
    bottomTab->addTab(m_varPanel, tr("变量"));
    bottomTab->setUsesScrollButtons(false);
    bottomTab->tabBar()->setExpanding(true);

    // 灵动岛：系统级独立顶层窗口，浮在屏幕顶部居中（不嵌在界面里）
    m_island = new DynamicIsland();   // 无 parent：独立窗口，不跟随 MainWindow 最小化
    // 执行边框：全屏置顶发光线框，运行流程时显示（初始隐藏）
    m_runBorder = new RunBorder();
    bottomTab->setMinimumHeight(240);
    midSplit->addWidget(bottomTab);
    midSplit->setStretchFactor(0, 3);
    midSplit->setStretchFactor(1, 2);
    midSplit->setSizes({ 480, 320 });

    hSplit->addWidget(midSplit);

    m_paramPanel = new ParamPanel(&m_flow, hSplit);
    hSplit->addWidget(m_paramPanel);

    hSplit->setStretchFactor(0, 0);
    hSplit->setStretchFactor(1, 1);
    hSplit->setStretchFactor(2, 0);
    hSplit->setSizes({ 190, 900, 300 });

    auto* vlay = new QVBoxLayout(central);
    vlay->setContentsMargins(10, 10, 10, 10);
    vlay->addWidget(hSplit);
    setCentralWidget(central);

    // 画布选中 → 参数面板
    connect(m_canvas->canvasScene(), &FlowCanvasScene::nodeSelected,
            m_paramPanel, &ParamPanel::setNode);
    connect(m_canvas->canvasScene(), &FlowCanvasScene::requestRunFrom,
            this, &MainWindow::runCurrent);   // 右键"从此运行"
    connect(m_paramPanel, &ParamPanel::paramEdited, this, [this](const QString& id) {
        m_canvas->canvasScene()->refreshNodeSummary(id);
        updateTitle();   // 参数写回会置 dirty，刷新标题栏 *
    });

    // 状态栏：主提示 + 缩放比例 + 节点数 + 运行状态
    m_status = new QLabel(tr("就绪"));
    statusBar()->addWidget(m_status, 1);
    m_nodeCountLabel = new QLabel();
    statusBar()->addPermanentWidget(m_nodeCountLabel);
    m_zoomLabel = new QLabel();
    statusBar()->addPermanentWidget(m_zoomLabel);
    m_runStateLabel = new QLabel(tr("空闲"));
    statusBar()->addPermanentWidget(m_runStateLabel);

    updateNodeCount();
    onZoomChanged(m_canvas->zoomLevel());
    onRunningChanged(false);   // 初始化运行/停止可用性 + 动态 tooltip

    // 初始焦点放到画布，避免搜索框自动聚焦显示蓝色边框
    m_canvas->setFocus();
}

QString MainWindow::selectedNodeId() const {
    for (auto* it : m_canvas->canvasScene()->selectedItems()) {
        if (auto* ni = dynamic_cast<NodeItem*>(it)) return ni->nodeId();
    }
    return QString();
}

void MainWindow::openSettings() {
    SettingsDialog dlg(m_themeToggle, this);
    dlg.exec();
}

void MainWindow::showTutorialIfNeeded() {
    QSettings settings("AutoFlow", "AutoFlow");
    if (settings.value("tutorial/dontShow", false).toBool()) return;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("新手教程"));
    auto* lay = new QVBoxLayout(&dlg);

    // 欢迎引导文本
    auto* text = new QLabel(
        tr("欢迎使用 AutoFlow！\n\n"
           "1. 从左侧指令面板拖拽指令到画布\n"
           "2. 连接节点，编排执行顺序\n"
           "3. 点击“运行”（或按 F10）执行流程\n\n"
           "全局热键：F10 运行 / F12 停止"), &dlg);
    text->setWordWrap(true);
    lay->addWidget(text);

    auto* noShowBox = new QCheckBox(tr("不再提示"), &dlg);
    lay->addWidget(noShowBox);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
    lay->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

    dlg.exec();

    // 勾选后保存标记，下次启动不再弹出
    if (noShowBox->isChecked())
        settings.setValue("tutorial/dontShow", true);
}

void MainWindow::newFlow() {
    if (!confirmDiscardChanges()) return;
    m_flow = FlowModel();
    m_flow.name = tr("未命名流程").toStdString();
    m_flow.setDirty(false);   // 新建空流程视为干净基线（画布完全空白，不自动放开始节点）
    m_currentPath.clear();
    m_canvas->canvasScene()->loadFromModel();
    m_logPanel->clearLog();
    m_varPanel->clearVars();
    updateTitle();
    updateNodeCount();
    m_status->setText(tr("已新建流程，从左侧拖入指令开始编排"));
}

void MainWindow::applyTemplate(int index) {
    const auto& tpls = presetTemplates();
    if (index < 0 || index >= (int)tpls.size()) return;
    const PresetTemplate& tpl = tpls[index];
    const QString tplName = tr(tpl.name.toUtf8().constData());

    // 只允许在空画布（仅有默认 start 节点）上应用，避免覆盖已有编排
    if (!(m_flow.nodes.empty() ||
          (m_flow.nodes.size() == 1 && m_flow.nodes[0].instr == "start" && m_flow.edges.empty()))) {
        QMessageBox::information(this, tr("无法应用模板"),
                                 tr("请先「新建」清空画布，再应用模板。"));
        return;
    }

    // 重建模型：节点 + 连线
    m_flow = FlowModel();
    m_flow.name = tplName.toStdString();
    std::vector<std::string> ids;
    ids.reserve(tpl.nodes.size());
    for (const auto& tn : tpl.nodes) {
        FlowNode n;
        n.instr  = tn.instr;
        n.x      = tn.x;
        n.y      = tn.y;
        n.params = tn.params;
        n.id     = (tn.instr == "start") ? "start" : m_flow.newId();
        ids.push_back(n.id);
        m_flow.addNode(std::move(n));
    }
    for (const auto& te : tpl.edges) {
        FlowEdge e;
        e.from  = ids[te.from];
        e.to    = ids[te.to];
        e.label = te.label;
        m_flow.addEdge(std::move(e));
    }

    m_currentPath.clear();
    m_canvas->canvasScene()->loadFromModel();
    m_logPanel->clearLog();
    m_varPanel->clearVars();
    updateTitle();
    updateNodeCount();
    m_status->setText(tr("已应用模板：") + tplName);
}

void MainWindow::openTemplateDialog() {
    // 由预设模板构造对话框条目：名称/描述翻译显示，节点链按指令元信息拼接
    QVector<TemplateItem> items;
    items.reserve((int)presetTemplates().size());
    for (const auto& tpl : presetTemplates()) {
        TemplateItem item;
        item.name = tr(tpl.name.toUtf8().constData());
        item.desc = tr(tpl.desc.toUtf8().constData());
        QStringList chain;
        for (const auto& tn : tpl.nodes) {
            const IInstruction* ii = InstructionRegistry::instance().get(tn.instr);
            if (!ii) continue;   // 未注册的指令 id：跳过
            chain << trInstr(ii->meta().name.c_str());
        }
        item.chain = chain.join(QStringLiteral(" → "));
        items.append(std::move(item));
    }

    TemplateDialog dlg(items, this);
    connect(&dlg, &TemplateDialog::newTemplateRequested, this, [this] { newTemplate(); });
    connect(&dlg, &TemplateDialog::openTemplateRequested, this, [this] { openTemplate(); });
    connect(&dlg, &TemplateDialog::saveTemplateRequested, this, [this] { saveTemplate(); });
    if (dlg.exec() == QDialog::Accepted && dlg.selectedIndex() >= 0)
        applyTemplate(dlg.selectedIndex());
}

// 用户模板目录：AppData/AutoFlow/templates
static QString templatesDir() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) dir = QDir::homePath() + QStringLiteral("/AutoFlow");
    dir += QStringLiteral("/templates");
    QDir().mkpath(dir);
    return dir;
}

void MainWindow::newTemplate() {
    if (m_flow.nodes.empty()) {
        m_status->setText(tr("流程为空，无法保存为模板"));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, tr("新建模板"),
        templatesDir() + QStringLiteral("/未命名模板.json"),
        tr("AutoFlow 模板 (*.json)"));
    if (path.isEmpty()) return;
    std::string err;
    if (!m_flow.saveToFile(path.toStdString(), err)) {
        QMessageBox::warning(this, tr("保存失败"), QString::fromStdString(err));
        return;
    }
    m_status->setText(tr("已保存模板：") + QFileInfo(path).fileName());
}

void MainWindow::openTemplate() {
    const QString path = QFileDialog::getOpenFileName(this, tr("打开模板"),
        templatesDir(), tr("AutoFlow 模板 (*.json)"));
    if (path.isEmpty()) return;
    openFlowFromPath(path);
}

void MainWindow::saveTemplate() {
    if (m_currentPath.isEmpty()) { newTemplate(); return; }
    saveFlow();
}

void MainWindow::openFlow() {
    if (!confirmDiscardChanges()) return;
    // 起始目录：最近使用目录 → 默认目录 → 原行为（当前文件路径）
    QString startDir = Settings::instance().lastDir();
    if (startDir.isEmpty()) startDir = Settings::instance().filesDefaultDir();
    if (startDir.isEmpty()) startDir = m_currentPath;
    QString path = QFileDialog::getOpenFileName(this, tr("打开流程"), startDir,
                                                tr("AutoFlow 流程 (*.json)"));
    if (path.isEmpty()) return;
    openFlowFromPath(path);
}

void MainWindow::openFlowFromPath(const QString& path) {
    std::string err;
    if (!m_flow.loadFromFile(path.toStdString(), err)) {
        QMessageBox::warning(this, tr("打开失败"), QString::fromStdString(err));
        return;
    }
    m_currentPath = path;
    m_canvas->canvasScene()->loadFromModel();
    m_logPanel->clearLog();
    updateTitle();
    updateNodeCount();

    // 打开成功：记住所在目录，并把文件提到最近列表最前（去重、截断到 recentMax）
    Settings& s = Settings::instance();
    s.setLastDir(QFileInfo(path).absolutePath());
    QStringList recent = s.recentFiles();
    recent.removeAll(path);
    recent.prepend(path);
    const int max = qMax(1, s.recentMax());
    while (recent.size() > max) recent.removeLast();
    s.setRecentFiles(recent);
}

void MainWindow::rebuildRecentMenu() {
    if (!m_recentMenu) return;
    m_recentMenu->clear();
    const QStringList recent = Settings::instance().recentFiles();
    if (recent.isEmpty()) {
        QAction* none = m_recentMenu->addAction(tr("（无最近文件）"));
        none->setEnabled(false);
        return;
    }
    for (const QString& path : recent) {
        QAction* a = m_recentMenu->addAction(QFileInfo(path).fileName(), this,
                                             [this, path] { openFlowFromPath(path); });
        a->setToolTip(path);   // 完整路径放 tooltip，列表只显示文件名
    }
}

void MainWindow::saveFlow() {
    if (m_currentPath.isEmpty()) { saveFlowAs(); return; }
    std::string err;
    if (!m_flow.saveToFile(m_currentPath.toStdString(), err)) {
        QMessageBox::warning(this, tr("保存失败"), QString::fromStdString(err));
        return;
    }
    m_flow.setDirty(false);
    updateTitle();
    m_status->setText(tr("已保存: ") + m_currentPath);
}

void MainWindow::saveFlowAs() {
    // 起始目录：最近使用目录 → 默认目录 → 原行为（当前文件路径）
    QString startDir = Settings::instance().lastDir();
    if (startDir.isEmpty()) startDir = Settings::instance().filesDefaultDir();
    if (startDir.isEmpty()) startDir = m_currentPath;
    QString path = QFileDialog::getSaveFileName(this, tr("另存为"), startDir,
                                                tr("AutoFlow 流程 (*.json)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".json")) path += ".json";
    m_currentPath = path;
    saveFlow();
    updateTitle();
}

void MainWindow::runCurrent(const QString& startId) {
    if (m_engine.isRunning()) return;
    if (m_flow.nodes.empty()) { m_status->setText(tr("流程为空，请先拖入指令")); return; }

    // 运行前校验：全模型缺失项汇总（validateCurrent 仅返回 bool、无渲染，属死代码，已移除调用）
    const QStringList errors = collectValidationErrors();
    if (!errors.isEmpty()) {
        QStringList head = errors.mid(0, 8);
        QString detail = head.join("\n");
        if (errors.size() > head.size())
            detail += QStringLiteral("\n… ") + tr("等 %1 项").arg(errors.size());
        QMessageBox::warning(this, tr("参数不完整"),
                             tr("以下参数缺失或非法，请补全后再运行：\n\n%1").arg(detail));
        return;
    }

    m_logPanel->clearLog();
    m_varPanel->clearVars();
    m_record.clear();
    m_record.startedAt = QDateTime::currentMSecsSinceEpoch();
    m_canvas->canvasScene()->clearRunStates();
    m_canvas->canvasScene()->clearHighlight();

    m_engine.setFlow(m_flow);
    m_engine.startRun(startId);
    m_status->setText(tr("运行中…"));
}

void MainWindow::stopRun() { m_engine.stop(); }

void MainWindow::stepRun() {
    if (!m_engine.isRunning()) {
        m_engine.setStepMode(true);
        runCurrent(QString());
    } else {
        m_engine.stepOnce();
    }
}

void MainWindow::scheduleRun() {
    bool ok = false;
    int sec = QInputDialog::getInt(this, tr("定时运行"),
                                   tr("每隔多少秒自动运行一次当前流程？\n（输入 0 关闭定时）"),
                                   0, 0, 86400, 1, &ok);
    if (!ok) return;
    Scheduler::Task t;
    t.name = tr("定时任务");
    t.mode = Scheduler::EveryInterval;
    t.intervalSec = sec;
    t.enabled = sec > 0;
    m_scheduler.tasks().clear();
    m_scheduler.addTask(t);
    m_scheduler.setEnabled(sec > 0);
    m_status->setText(sec > 0 ? tr("已开启定时运行：每 %1 秒一次").arg(sec) : tr("已关闭定时运行"));
}

void MainWindow::onNodeStarted(const QString& id, const QString& name) {
    m_canvas->canvasScene()->highlightNode(id);
    m_canvas->canvasScene()->markNodeState(id, "running");
    m_logRowsAtStart = m_logPanel->rowCount();

    // AI 节点（调用耗时不确定）显示明确的"调用中"提示，其余显示运行中节点名
    bool isAi = false;
    if (FlowNode* n = m_flow.nodeById(id.toStdString())) {
        const IInstruction* ii = InstructionRegistry::instance().get(n->instr);
        isAi = ii && ii->meta().category == Category::AI;
    }
    m_status->setText(isAi ? tr("正在调用 AI 服务：%1…").arg(name)
                           : tr("运行中：%1…").arg(name));
}

void MainWindow::onNodeFinished(const QString& id, bool ok, qint64 ms, const QString& error) {
    RunStepResult r;
    r.nodeId = id;
    r.ok = ok;
    r.elapsedMs = ms;
    r.error = error;
    FlowNode* n = m_flow.nodeById(id.toStdString());
    if (n) {
        const IInstruction* ii = InstructionRegistry::instance().get(n->instr);
        if (ii) r.nodeName = trInstr(ii->meta().name.c_str());
    }
    m_record.addStep(r);
    // 运行结果可视化：节点标绿(完成)/标红(失败)，保留到下次运行前由 clearRunStates 清
    m_canvas->canvasScene()->markNodeState(id, ok ? "ok" : "error");
    if (ok) {
        // 一步一条日志：成功时把耗时回填到该节点最后一条消息；若该节点没打日志则补一条
        if (m_logPanel->rowCount() == m_logRowsAtStart)
            m_logPanel->appendLog("success", id, r.nodeName, tr("完成"), ms);
        else
            m_logPanel->setLastTiming(ms);
    } else {
        m_logPanel->appendLog("error", id, r.nodeName, tr("失败: ") + error, ms);
    }
}

void MainWindow::onLog(const QString& level, const QString& nodeId, const QString& nodeName, const QString& text) {
    m_record.addLog(level, nodeId, nodeName, text);
    m_logPanel->appendLog(level, nodeId, nodeName, text);
}

void MainWindow::onRunFinished(bool ok, const QString& summary) {
    m_engine.setStepMode(false);
    m_canvas->canvasScene()->clearHighlight();
    m_record.finishedAt = QDateTime::currentMSecsSinceEpoch();
    m_record.success = ok;
    m_status->setText(summary);

    qint64 total = m_record.finishedAt - m_record.startedAt;
    m_logPanel->appendLog(ok ? "success" : "error", "", "",
                          summary + tr(" · 总耗时 ") + QString::number(total) + " ms", total);

    // 运行完成通知（设置开启时）：无托盘图标，用系统提示音 + 状态栏提示（非阻塞）
    if (Settings::instance().notifyOnFinish()) {
        QApplication::beep();
        m_status->setText(tr("运行完成 — ") + summary);
    }
}

void MainWindow::updateTitle() {
    QString name = QString::fromStdString(m_flow.name);
    if (m_flow.isDirty()) name += "*";   // 未保存改动标记
    QString full = tr("AutoFlow 可视化自动化工具 — %1%2  [构建 %3 %4]")
                       .arg(name)
                       .arg(m_currentPath.isEmpty() ? "" : ("  [" + m_currentPath + "]"))
                       .arg(QString::fromLatin1(__DATE__))
                       .arg(QString::fromLatin1(__TIME__));
    setWindowTitle(full);
    if (m_titleBar) {
        m_titleBar->setTitle(QString("AutoFlow — %1").arg(name));
    }
}

void MainWindow::toggleMaximize() {
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void MainWindow::closeEvent(QCloseEvent* e) {
    if (!confirmDiscardChanges()) {
        e->ignore();
        return;
    }
    // 关闭时保存窗口几何（位置/尺寸/最大化状态），下次启动恢复
    QSettings("AutoFlow", "AutoFlow").setValue("window/geometry", saveGeometry());
    QMainWindow::closeEvent(e);
}

bool MainWindow::confirmDiscardChanges() {
    if (!m_flow.isDirty()) return true;
    QMessageBox box(this);
    box.setWindowTitle(tr("未保存的改动"));
    box.setText(tr("当前流程有未保存的改动。"));
    box.setInformativeText(tr("是否在继续前保存？"));
    QPushButton* saveBtn = box.addButton(tr("保存"), QMessageBox::AcceptRole);
    QAbstractButton* discardBtn = box.addButton(tr("不保存"), QMessageBox::DestructiveRole);
    box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.setDefaultButton(saveBtn);
    box.exec();
    const QAbstractButton* clicked = box.clickedButton();
    if (clicked == saveBtn) {
        saveFlow();
        return !m_flow.isDirty();   // 保存成功 → dirty 清空；另存被取消 → 仍 dirty
    }
    return clicked == discardBtn;
}

QStringList MainWindow::collectValidationErrors() const {
    QStringList errors;
    for (const auto& n : m_flow.nodes) {
        if (!n.enabled) continue;   // 禁用节点运行时会跳过，不参与校验
        const IInstruction* instr = InstructionRegistry::instance().get(n.instr);
        if (!instr) continue;
        const QString nodeName = QStringLiteral("%1 (%2)").arg(
            trInstr(instr->meta().name.c_str()), QString::fromStdString(n.id));
        for (const auto& p : instr->meta().params) {
            if (!p.required) continue;

            // if 条件判断的快捷预设：非「自定义」时，部分参数由预设自动填充，无需用户填写
            if (n.instr == "if") {
                std::string preset = "自定义";
                auto pit = n.params.find("preset");
                if (pit != n.params.end() && pit.value().is_string())
                    preset = pit.value().get<std::string>();
                const bool autoLeftRight = (preset == "找图成功" || preset == "找图失败"
                    || preset == "AI 识别到目标" || preset == "AI 未识别到目标");
                const bool autoLeftOnly = (preset == "OCR 识别到文字"
                    || preset == "软件已打开" || preset == "软件未打开");
                if (autoLeftRight && (p.key == "left" || p.key == "right")) continue;
                if (autoLeftOnly && p.key == "left") continue;
            }

            bool empty = false;
            auto it = n.params.find(p.key);
            if (it != n.params.end()) {
                const json& v = it.value();
                if (v.is_string()) empty = v.get<std::string>().empty();
                // number/bool/array/object 视为有值
            } else {
                empty = p.def.empty();   // 参数缺省时按默认值判断（与执行期一致）
            }
            if (empty)
                errors << QStringLiteral("%1：%2").arg(nodeName, trInstr(p.label.c_str()));
        }
    }
    return errors;
}

void MainWindow::updateNodeCount() {
    if (m_nodeCountLabel)
        m_nodeCountLabel->setText(tr("节点 %1").arg(m_flow.nodes.size()));
}

void MainWindow::updateActionStates() {
    const bool running = m_engine.isRunning();
    if (m_runAction) m_runAction->setEnabled(!running);
    if (m_stopAction) m_stopAction->setEnabled(running);
    if (m_newAction) m_newAction->setEnabled(!running);
    if (m_openAction) m_openAction->setEnabled(!running);
    if (m_templateAction) m_templateAction->setEnabled(!running);
    if (m_undoAction && m_canvas)
        m_undoAction->setEnabled(m_canvas->canvasScene()->canUndo());
    if (m_redoAction && m_canvas)
        m_redoAction->setEnabled(m_canvas->canvasScene()->canRedo());
}

void MainWindow::onRunningChanged(bool running) {
    // 运行/结束时显示/隐藏屏幕边框
    if (m_runBorder) {
        if (running) m_runBorder->showOnScreen();
        else m_runBorder->hide();
    }

    updateActionStates();
    if (m_runStateLabel)
        m_runStateLabel->setText(running ? tr("运行中") : tr("空闲"));

    // tooltip 动态取热键，不写死 F10/F12
    const QString runKey = Settings::instance().hotkeyRun();
    const QString stopKey = Settings::instance().hotkeyStop();
    if (m_runAction)
        m_runAction->setToolTip(tr("运行流程（%1 全局热键）").arg(runKey));
    if (m_stopAction)
        m_stopAction->setToolTip(running ? tr("运行中，点击停止")
                                         : tr("停止运行（%1 全局热键）").arg(stopKey));
}

void MainWindow::onZoomChanged(qreal level) {
    if (m_zoomLabel)
        m_zoomLabel->setText(tr("缩放 %1%").arg(qRound(level * 100)));
}

void MainWindow::onCanvasModelChanged() {
    updateTitle();
    updateNodeCount();
    if (m_undoAction && m_canvas)
        m_undoAction->setEnabled(m_canvas->canvasScene()->canUndo());
    if (m_redoAction && m_canvas)
        m_redoAction->setEnabled(m_canvas->canvasScene()->canRedo());
}

void MainWindow::applyAutostart(bool on) {
    // 通过 QSettings NativeFormat 直接读写注册表 Run 键
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    if (on) {
        // 写当前 exe 全路径（统一为反斜杠并加引号，兼容含空格路径）
        QString exe = QCoreApplication::applicationFilePath();
        exe.replace('/', '\\');
        reg.setValue("AutoFlow", QString("\"%1\"").arg(exe));
    } else {
        reg.remove("AutoFlow");
    }
}

// 无边框窗口的边缘缩放：让鼠标在窗口边缘拖动时触发系统缩放
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef Q_OS_WIN
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_NCHITTEST && !isMaximized()) {
        const int border = 6;
        QPoint pos = QCursor::pos();
        QRect r = frameGeometry();
        bool left   = pos.x() <  r.left()   + border;
        bool right  = pos.x() >= r.right()  - border;
        bool top    = pos.y() <  r.top()    + border;
        bool bottom = pos.y() >= r.bottom() - border;
        if (left && top)         *result = HTTOPLEFT;
        else if (right && top)   *result = HTTOPRIGHT;
        else if (left && bottom) *result = HTBOTTOMLEFT;
        else if (right && bottom)*result = HTBOTTOMRIGHT;
        else if (left)           *result = HTLEFT;
        else if (right)          *result = HTRIGHT;
        else if (top)            *result = HTTOP;
        else if (bottom)         *result = HTBOTTOM;
        else return false;
        return true;
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

} // namespace autoflow
