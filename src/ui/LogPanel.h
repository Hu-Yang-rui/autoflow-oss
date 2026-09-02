#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QFile>

class QComboBox;
class QLabel;
class QPushButton;

namespace autoflow {

// 底部运行日志面板
class LogPanel : public QWidget {
    Q_OBJECT
public:
    explicit LogPanel(QWidget* parent = nullptr);

public slots:
    void appendLog(const QString& level, const QString& nodeId, const QString& nodeName,
                   const QString& text, qint64 elapsedMs = 0);
    void clearLog();

    int rowCount() const { return m_table->rowCount(); }
    void setLastTiming(qint64 ms);   // 把耗时回填到最后一行（用于“一步一条日志”）

private:
    // 可选落盘日志：懒创建/打开日志文件并追加一行
    void appendLogToFile(const QString& level, const QString& nodeName, const QString& text);
    void applyLevelFilter();
    void copySelectedLog();
    void applyStyle();          // 依当前主题刷新标题/ghost 按钮/表格内联样式
    void refreshLevelColors();  // 主题切换后重刷已有行的级别着色

    QTableWidget* m_table = nullptr;
    QLabel* m_title = nullptr;
    QPushButton* m_clearBtn = nullptr;
    QPushButton* m_copyBtn = nullptr;
    QComboBox* m_levelFilter = nullptr;
    QFile m_logFile;   // 懒打开（Settings::logToFile() 为 true 时）
};

} // namespace autoflow
