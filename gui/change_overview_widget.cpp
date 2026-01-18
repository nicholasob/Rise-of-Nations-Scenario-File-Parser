#include "change_overview_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace {
constexpr int ROLE_OFFSET = Qt::UserRole + 1;
constexpr int ROLE_LENGTH = Qt::UserRole + 2;
}

ChangeOverviewWidget::ChangeOverviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_tree(new QTreeWidget(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tree->setColumnCount(4);
    m_tree->setHeaderLabels(QStringList() << "Chunk" << "Field" << "Old" << "New");
    m_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tree->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tree->setRootIsDecorated(false);
    m_tree->setAlternatingRowColors(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_tree, &QTreeWidget::itemActivated,
            this, &ChangeOverviewWidget::handleItemActivated);
    connect(m_tree, &QTreeWidget::itemClicked,
            this, &ChangeOverviewWidget::handleItemActivated);

    layout->addWidget(m_tree);
}

void ChangeOverviewWidget::setChanges(const QVector<FieldChange>& changes)
{
    m_tree->clear();

    for (const auto& change : changes) {
        auto *item = new QTreeWidgetItem(m_tree);
        item->setText(0, QString("%1 (%2)").arg(change.chunkName, change.chunkPath));
        item->setText(1, change.fieldName);
        item->setText(2, change.oldValue);
        item->setText(3, change.newValue);
        item->setData(0, ROLE_OFFSET, QVariant::fromValue(change.offset));
        item->setData(0, ROLE_LENGTH, QVariant::fromValue(change.length));
    }

    if (changes.isEmpty()) {
        auto *item = new QTreeWidgetItem(m_tree);
        item->setText(0, "No changes detected");
        item->setFlags(Qt::NoItemFlags);
    }
}

void ChangeOverviewWidget::clear()
{
    m_tree->clear();
}

void ChangeOverviewWidget::handleItemActivated(QTreeWidgetItem *item, int /*column*/)
{
    if (!item) {
        return;
    }
    const auto offsetVar = item->data(0, ROLE_OFFSET);
    const auto lengthVar = item->data(0, ROLE_LENGTH);
    if (!offsetVar.isValid() || !lengthVar.isValid()) {
        return;
    }
    emit highlightRequested(offsetVar.toULongLong(), lengthVar.toULongLong());
}
