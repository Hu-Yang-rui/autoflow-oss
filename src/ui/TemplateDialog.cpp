#include "TemplateDialog.h"
#include "ThemeManager.h"
#include "Palette.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPushButton>

namespace autoflow {

TemplateDialog::TemplateDialog(const QVector<TemplateItem>& items, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("选择模板"));

    auto* lay = new QVBoxLayout(this);
    lay->setSpacing(8);

    // 顶部工具条：模板新建 / 打开 / 保存（由 MainWindow 处理，这里仅转发信号）
    auto* toolbar = new QHBoxLayout();
    auto* newBtn  = new QPushButton(tr("模板新建"), this);
    auto* openBtn = new QPushButton(tr("模板打开"), this);
    auto* saveBtn = new QPushButton(tr("模板保存"), this);
    connect(newBtn,  &QPushButton::clicked, this, [this] { emit newTemplateRequested(); });
    connect(openBtn, &QPushButton::clicked, this, [this] { emit openTemplateRequested(); });
    connect(saveBtn, &QPushButton::clicked, this, [this] { emit saveTemplateRequested(); });
    toolbar->addWidget(newBtn);
    toolbar->addWidget(openBtn);
    toolbar->addWidget(saveBtn);
    toolbar->addStretch(1);
    lay->addLayout(toolbar);

    const bool dark = ThemeManager::instance().effectiveDark();

    // 模板列表（QListWidget，文字由 Qt 标准渲染，可靠）
    auto* list = new QListWidget(this);
    list->setObjectName("tplList");
    list->setStyleSheet(QString(
        "QListWidget#tplList { background: %1; border: 1px solid %2; border-radius: 6px; }"
        "QListWidget#tplList::item { padding: 10px 12px; border-bottom: 1px solid %3; }"
        "QListWidget#tplList::item:hover { background: %4; }"
        "QListWidget#tplList::item:selected { background: %4; color: %5; }")
        .arg(Palette::surface(dark).name(),
             Palette::border(dark).name(),
             Palette::border(dark).name(),
             Palette::surface2(dark).name(),
             Palette::accent(dark).name()));

    for (int i = 0; i < items.size(); ++i) {
        const TemplateItem& it = items[i];
        auto* wi = new QListWidgetItem(
            it.name + QStringLiteral("\n") + it.desc + QStringLiteral("\n") + it.chain, list);
        wi->setData(Qt::UserRole, i);
        wi->setSizeHint(QSize(0, 60));
    }
    // 单击仅选中（防误点替换画布），需点击“应用”确认后才会应用
    connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem* wi) {
        m_selected = wi->data(Qt::UserRole).toInt();
        if (auto* okBtn = findChild<QPushButton*>(QStringLiteral("applyTplBtn")))
            okBtn->setEnabled(true);
    });
    // 双击视为明确确认，可直接应用
    connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* wi) {
        m_selected = wi->data(Qt::UserRole).toInt();
        accept();
    });
    lay->addWidget(list);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    auto* applyBtn = buttons->addButton(tr("应用"), QDialogButtonBox::AcceptRole);
    applyBtn->setObjectName(QStringLiteral("applyTplBtn"));
    applyBtn->setEnabled(false);   // 未选中时禁用，避免空应用
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    connect(applyBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    lay->addWidget(buttons);

    setMinimumWidth(400);
    resize(440, 360);
}

} // namespace autoflow
