#pragma once
#include <QMainWindow>
#include "../core/FlowModel.h"
#include "../core/ExecutionEngine.h"
#include "../core/RunRecord.h"
#include "../core/Scheduler.h"

class QToolBar;
class QStatusBar;
class QLabel;
class QTabWidget;
class QAction;
class QSplitter;
class QMenu;
class QMenuBar;
class QCloseEvent;

namespace autoflow {

class InstructionPanel;
class FlowCanvas;
class ParamPanel;
class LogPanel;
class VariablePanel;
class HotkeyManager;
class ThemeToggle;
class TitleBar;
class DynamicIsland;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // 开机自启：写入/移除注册表 Run 键（HKCU\...\CurrentVersion\Run 的 "AutoFlow" 值）
    static void applyAutostart(bool on);

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void closeEvent(QCloseEvent* e) override;

private slots:
    void newFlow();
    void openFlow();
    void rebuildRecentMenu();        // 重建“最近打开”子菜单（aboutToShow 时调用）
    void applyTemplate(int index);   // 应用预设模板
    void saveFlow();
    void saveFlowAs();
    void runCurrent(const QString& startId = QString());
    void stopRun();
    void stepRun();
    void scheduleRun();

    void onNodeStarted(const QString& id, const QString& name);
    void onNodeFinished(const QString& id, bool ok, qint64 ms, const QString& error);
    void onLog(const QString& level, const QString& nodeId, const QString& nodeName, const QString& text);
    void onRunFinished(bool ok, const QString& summary);
    void onRunningChanged(bool running);   // 运行/停止状态联动
    void onZoomChanged(qreal level);       // 状态栏缩放比例
    void onCanvasModelChanged();           // 画布模型变化：更新脏标记/节点数/撤销可用性

private:
    void buildUi();
    void updateTitle();
    void toggleMaximize();
    QString selectedNodeId() const;
    void openSettings();          // 设置对话框
    void openTemplateDialog();    // 模板选择对话框
    void newTemplate();           // 把当前流程另存为模板（存到用户模板目录）
    void openTemplate();          // 打开模板文件（.json）
    void saveTemplate();          // 保存当前模板（无路径则另存）
    void showTutorialIfNeeded();  // 首次运行新手教程
    void openFlowFromPath(const QString& path);   // 打开指定路径的流程并更新最近文件/最近目录
    bool confirmDiscardChanges();               // dirty 时弹保存/不保存/取消，返回是否可继续
    QStringList collectValidationErrors() const; // 运行前汇总所有节点的缺失/非法参数
    void updateNodeCount();
    void updateActionStates();    // 依据运行状态与撤销栈刷新动作可用性

    FlowModel m_flow;
    ExecutionEngine m_engine;
    RunRecord m_record;
    Scheduler m_scheduler;
    QString m_currentPath;

    InstructionPanel* m_instrPanel = nullptr;
    FlowCanvas* m_canvas = nullptr;
    ParamPanel* m_paramPanel = nullptr;
    LogPanel* m_logPanel = nullptr;
    VariablePanel* m_varPanel = nullptr;
    HotkeyManager* m_hotkeys = nullptr;
    ThemeToggle* m_themeToggle = nullptr;
    TitleBar* m_titleBar = nullptr;
    DynamicIsland* m_island = nullptr;
    QLabel* m_status = nullptr;
    QLabel* m_zoomLabel = nullptr;
    QLabel* m_nodeCountLabel = nullptr;
    QLabel* m_runStateLabel = nullptr;
    QAction* m_runAction = nullptr;
    QAction* m_stopAction = nullptr;
    QAction* m_stepAction = nullptr;
    QAction* m_newAction = nullptr;
    QAction* m_openAction = nullptr;
    QAction* m_saveAction = nullptr;
    QAction* m_saveAsAction = nullptr;
    QAction* m_templateAction = nullptr;
    QAction* m_undoAction = nullptr;
    QAction* m_redoAction = nullptr;
    QAction* m_deleteAction = nullptr;
    QAction* m_selectAllAction = nullptr;
    QAction* m_copyAction = nullptr;
    QAction* m_pasteAction = nullptr;
    QAction* m_settingsAction = nullptr;
    QMenu* m_recentMenu = nullptr;
    bool m_restoringTheme = false;   // 启动时恢复主题：避免把 followSystemTheme 误关
    int m_logRowsAtStart = 0;   // 节点开始执行时的日志行数（用于回填耗时）
};

} // namespace autoflow
