#pragma once
#include <QObject>
#include <QApplication>
#include <QFont>

namespace autoflow {

// 主题管理器：加载 QSS 资源，支持深/浅切换
class ThemeManager : public QObject {
    Q_OBJECT
public:
    static ThemeManager& instance();

    // 返回已加载的 Smiley Sans（得意黑）字体（供条目视图等特殊控件显式设置）
    static QFont smileySansFont();

    bool isDark() const { return m_dark; }
    // 生效的深色状态：开启"跟随系统"时取 OS 主题，否则取显式开关 m_dark
    bool effectiveDark() const;
    void setDark(bool dark);

signals:
    void themeChanged(bool dark);

private:
    ThemeManager();
    void apply();
    bool m_dark = true;
    bool m_initialized = false;
};

} // namespace autoflow
