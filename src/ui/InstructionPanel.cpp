#include "InstructionPanel.h"
#include "ThemeManager.h"
#include "Palette.h"
#include "IconPainter.h"
#include "../instructions/InstructionRegistry.h"

#include <QHeaderView>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QHoverEvent>
#include <QStyle>

namespace autoflow {

// 分类名（中文源串，不受 i18n 影响）→ 统一图标
static IconPainter::Id categoryIcon(const QString& cat) {
    if (cat == QStringLiteral("图像")) return IconPainter::Id::CatImage;
    if (cat == QStringLiteral("键鼠")) return IconPainter::Id::CatInput;
    if (cat == QStringLiteral("数据")) return IconPainter::Id::CatData;
    if (cat == QStringLiteral("流程")) return IconPainter::Id::CatFlow;
    if (cat == QStringLiteral("AI"))   return IconPainter::Id::CatAI;
    if (cat == QStringLiteral("窗口")) return IconPainter::Id::CatWindow;
    if (cat == QStringLiteral("文件")) return IconPainter::Id::CatFile;
    if (cat == QStringLiteral("系统")) return IconPainter::Id::CatSystem;
    return IconPainter::Id::CatFlow;
}

// 完全手动绘制 item，不调用 Qt 的 drawControl，从而彻底不画任何虚线焦点框
class NoFocusDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const bool dark = ThemeManager::instance().effectiveDark();

        // 分类项（顶层，可展开的主模块）vs 指令项（子模块）
        const bool isCategory = !index.parent().isValid();
        const bool hasChildren = index.model() && index.model()->rowCount(index) > 0;
        bool expanded = false;
        if (const auto* tree = qobject_cast<const QTreeWidget*>(opt.widget))
            expanded = tree->isExpanded(index);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        // 背景
        if (isCategory) {
            // 分类项：浅色背景框，与指令项明显区分
            const QRectF r = QRectF(opt.rect).adjusted(1, 1, -1, -1);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Palette::surface2(dark));
            painter->drawRoundedRect(r, 4, 4);
            if (opt.state & QStyle::State_MouseOver) {
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(Palette::borderStrong(dark), 1.0));
                painter->drawRoundedRect(r, 4, 4);
            }
        } else if (opt.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
            // 指令项：选中/悬停 背景 + 边框
            const bool selected = opt.state & QStyle::State_Selected;
            const QRectF r = QRectF(opt.rect).adjusted(1, 1, -1, -1);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Palette::surface2(dark));
            painter->drawRoundedRect(r, 4, 4);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(selected ? Palette::accent(dark) : Palette::borderStrong(dark),
                                 selected ? 1.5 : 1.0));
            painter->drawRoundedRect(r, 4, 4);
        }

        // 分类项：左侧画分类图标，右侧画展开箭头（▾ 展开 / ▸ 折叠）；指令项文字缩进对齐
        int textLeft = 8;
        if (isCategory) {
            const QRectF iconBox(opt.rect.left() + 8, opt.rect.center().y() - 9, 18, 18);
            IconPainter::paint(*painter, iconBox,
                               categoryIcon(index.data(Qt::UserRole).toString()),
                               Palette::textDim(dark), 1.75);
            textLeft = 30;

            if (hasChildren) {
                const QRectF arrowBox(opt.rect.right() - 20, opt.rect.center().y() - 7, 14, 14);
                IconPainter::paint(*painter, arrowBox,
                                   expanded ? IconPainter::Id::ChevronDown : IconPainter::Id::ChevronRight,
                                   Palette::textDim(dark), 1.75);
            }
        }

        // 文字（分类项右侧给箭头留白）
        painter->setFont(opt.font);
        painter->setPen(Palette::text(dark));
        painter->drawText(opt.rect.adjusted(textLeft, 0, isCategory ? -24 : -8, 0),
                          Qt::AlignVCenter | Qt::AlignLeft, opt.text);

        painter->restore();
    }
};

InstructionTree::InstructionTree(QWidget* parent) : QTreeWidget(parent) {
    setHeaderHidden(true);
    setIndentation(10);
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragDropMode(QAbstractItemView::DragOnly);
    setObjectName("instructionPanel");
    setFont(ThemeManager::smileySansFont());
    header()->setFont(ThemeManager::smileySansFont());
    setFocusPolicy(Qt::NoFocus);   // 去掉点击后的焦点虚框（点击展开/拖拽都不需要键盘焦点）
    viewport()->setFocusPolicy(Qt::NoFocus);   // viewport 才是实际接收点击焦点的控件，也要禁用
    setItemDelegate(new NoFocusDelegate(this));   // 彻底去掉 current item 的虚线焦点框

    auto& reg = InstructionRegistry::instance();

    QFont catFont = font();
    catFont.setPixelSize(13);

    for (auto& cat : reg.categories()) {
        // 分类名/指令名/描述：std::string 源串不变，显示时按 "Instructions" 上下文翻译
        QTreeWidgetItem* catItem = new QTreeWidgetItem({ trInstr(cat.c_str()) });
        catItem->setData(0, Qt::UserRole, QString::fromStdString(cat));  // 存原始分类名，供图标映射
        catItem->setFont(0, catFont);
        catItem->setFlags(Qt::ItemIsEnabled);
        addTopLevelItem(catItem);

        for (auto* instr : reg.byCategory(cat)) {
            auto m = instr->meta();
            QString label = trInstr(m.name.c_str());
            QTreeWidgetItem* item = new QTreeWidgetItem(catItem, { label });
            item->setData(0, Qt::UserRole, QString::fromStdString(m.id));
            item->setData(0, Qt::UserRole + 1, trInstr(m.desc.c_str()));   // 描述，供搜索匹配
            item->setToolTip(0, trInstr(m.desc.c_str()));
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        }
        catItem->setExpanded(true);
    }

    // 修复：点击分类项整行即可展开/折叠，不必非点箭头
    connect(this, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int) {
        if (item && item->parent() == nullptr && item->childCount() > 0)
            item->setExpanded(!item->isExpanded());
    });
}

QStringList InstructionTree::mimeTypes() const {
    return { "application/x-autoflow-instr" };
}

QMimeData* InstructionTree::mimeData(const QList<QTreeWidgetItem*>& items) const {
    if (items.isEmpty()) return nullptr;
    QTreeWidgetItem* it = items.first();
    if (!it || it->parent() == nullptr) return nullptr;   // 分类头不可拖
    QMimeData* md = new QMimeData();
    md->setData("application/x-autoflow-instr", it->data(0, Qt::UserRole).toString().toUtf8());
    return md;
}

// 不画任何展开箭头/树线（分类作为纯标题展示）
void InstructionTree::drawBranches(QPainter*, const QRect&, const QModelIndex&) const {
}

// 手动绘制整行：branches（空）+ item（delegate），跳过 QTreeView 默认在
// current item 上画的虚线 focus rect（即用户看到的那个蓝色虚线框）
void InstructionTree::drawRow(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    opt.state &= ~QStyle::State_HasFocus;   // 去虚线焦点框
    // QTreeView 默认 drawRow 会根据 selectionModel 补 State_Selected、hover 补 State_MouseOver，这里手动补上
    if (selectionModel()->isSelected(index))
        opt.state |= QStyle::State_Selected;
    // hover：用成员变量 m_hoverIndex 判断（viewportEvent 的 HoverMove/HoverLeave 更新它，不依赖 QCursor::pos()）
    if (index == m_hoverIndex)
        opt.state |= QStyle::State_MouseOver;
    drawBranches(painter, opt.rect, index);
    itemDelegateForIndex(index)->paint(painter, opt, index);
}

bool InstructionTree::viewportEvent(QEvent* event) {
    if (event->type() == QEvent::HoverMove) {
        auto* he = static_cast<QHoverEvent*>(event);
        QModelIndex newHover = indexAt(he->position().toPoint());
        if (newHover != m_hoverIndex) {
            m_hoverIndex = newHover;
            viewport()->update();
        }
    } else if (event->type() == QEvent::HoverLeave) {
        if (m_hoverIndex.isValid()) {
            m_hoverIndex = QModelIndex();
            viewport()->update();
        }
    }
    return QTreeWidget::viewportEvent(event);
}

InstructionPanel::InstructionPanel(QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    auto* title = new QLabel(tr("指令面板"));
    title->setObjectName("panelTitle");
    lay->addWidget(title);

    auto* hint = new QLabel(tr("将指令拖拽到画布即可添加"));
    hint->setObjectName("panelHint");
    hint->setWordWrap(true);
    lay->addWidget(hint);

    // 搜索框：过滤指令
    auto* search = new QLineEdit();
    search->setObjectName("instrSearch");
    search->setPlaceholderText(tr("搜索指令…"));
    search->setClearButtonEnabled(true);
    lay->addWidget(search);
    connect(search, &QLineEdit::textChanged, this, [this](const QString& text) {
        QString t = text.trimmed();
        for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
            QTreeWidgetItem* cat = m_tree->topLevelItem(i);
            int visible = 0;
            for (int j = 0; j < cat->childCount(); ++j) {
                QTreeWidgetItem* child = cat->child(j);
                const bool match = t.isEmpty()
                    || child->text(0).contains(t, Qt::CaseInsensitive)
                    || child->data(0, Qt::UserRole + 1).toString().contains(t, Qt::CaseInsensitive);
                child->setHidden(!match);
                if (match) ++visible;
            }
            cat->setHidden(!t.isEmpty() && visible == 0);
            if (!t.isEmpty()) cat->setExpanded(true);
        }
    });

    m_tree = new InstructionTree(this);
    lay->addWidget(m_tree);
    setMinimumWidth(180);
}

} // namespace autoflow
