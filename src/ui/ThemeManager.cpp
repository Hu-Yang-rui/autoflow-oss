#include "ThemeManager.h"

#include <QFile>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>

#include "../core/Settings.h"

namespace autoflow {

// 探测 OS 当前是否为深色主题：
// 优先 QStyleHints::colorScheme()(Qt 6.5+)，Unknown 时回退到窗口背景亮度判断。
static bool systemDark() {
    const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();
    if (scheme == Qt::ColorScheme::Unknown)
        return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
    return scheme == Qt::ColorScheme::Dark;
}

ThemeManager::ThemeManager() {
    // 跟随系统模式下，OS 深浅色切换时实时换肤
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, [this](Qt::ColorScheme) {
        if (Settings::instance().followSystemTheme()) {
            apply();
            emit themeChanged(effectiveDark());
        }
    });
}

ThemeManager& ThemeManager::instance() {
    static ThemeManager tm;
    return tm;
}

QFont ThemeManager::smileySansFont() {
    QFont f;
    // 微软雅黑 UI（Regular，无 Light fallback 问题，所有字一致）
    f.setFamilies({ QStringLiteral("Microsoft YaHei UI"), QStringLiteral("Segoe UI") });
    // 不设 weight（默认 Regular 400，避免 Light 字重导致部分汉字 fallback 到 Regular 造成不一致）
    // 应用字号缩放（百分比，默认 100）；默认构造的字体 pointSizeF 为 -1，此处不改动
    const int scale = Settings::instance().fontScale();
    if (scale != 100 && f.pointSizeF() > 0)
        f.setPointSizeF(f.pointSizeF() * scale / 100.0);
    return f;
}

bool ThemeManager::effectiveDark() const {
    return Settings::instance().followSystemTheme() ? systemDark() : m_dark;
}

void ThemeManager::setDark(bool dark) {
    if (m_dark == dark && m_initialized) return;
    m_dark = dark;
    m_initialized = true;
    apply();
    emit themeChanged(effectiveDark());
}

void ThemeManager::apply() {
    QString path = effectiveDark() ? ":/style_dark.qss" : ":/style_light.qss";
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

} // namespace autoflow
