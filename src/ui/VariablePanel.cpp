#include "VariablePanel.h"
#include "ThemeManager.h"
#include "../core/Variable.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QCoreApplication>

namespace autoflow {

static QString typeName(const Variable& v) {
    // 非 QObject 自由函数：用 "VariablePanel" 上下文翻译
    switch (v.type) {
        case VarType::String: return QCoreApplication::translate("VariablePanel", "字符串");
        case VarType::Number: return QCoreApplication::translate("VariablePanel", "数字");
        case VarType::Bool:   return QCoreApplication::translate("VariablePanel", "布尔");
        case VarType::List:   return QCoreApplication::translate("VariablePanel", "列表");
        case VarType::Object: return QCoreApplication::translate("VariablePanel", "对象");
        default: return QCoreApplication::translate("VariablePanel", "空");
    }
}

VariablePanel::VariablePanel(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget(this);
    m_tree->setObjectName("varTree");
    m_tree->setColumnCount(3);
    m_tree->setHeaderLabels({ tr("变量名"), tr("类型"), tr("值") });
    m_tree->header()->setStretchLastSection(true);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tree->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tree->setFont(ThemeManager::smileySansFont());
    m_tree->header()->setFont(ThemeManager::smileySansFont());
    lay->addWidget(m_tree);
}

void VariablePanel::setSnapshot(const QString& jsonStr) {
    m_tree->clear();
    try {
        json j = json::parse(jsonStr.toStdString());
        VariableSystem vs;
        vs.fromJson(j);
        for (auto& kv : vs.all()) {
            auto* item = new QTreeWidgetItem({
                QString::fromStdString(kv.first),
                typeName(kv.second),
                QString::fromStdString(kv.second.toString())
            });
            m_tree->addTopLevelItem(item);
        }
    } catch (...) {}
}

void VariablePanel::clearVars() { m_tree->clear(); }

} // namespace autoflow
