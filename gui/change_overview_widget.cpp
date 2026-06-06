#include "change_overview_widget.h"
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QShortcut>

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
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_tree, &QTreeWidget::itemActivated,
            this, &ChangeOverviewWidget::handleItemActivated);
    connect(m_tree, &QTreeWidget::itemClicked,
            this, &ChangeOverviewWidget::handleItemActivated);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &ChangeOverviewWidget::showContextMenu);

    auto *copyShortcut = new QShortcut(QKeySequence::Copy, m_tree);
    connect(copyShortcut, &QShortcut::activated,
            this, &ChangeOverviewWidget::copySelectedRows);

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

void ChangeOverviewWidget::showContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    QAction *copySelected = menu.addAction(tr("Copy Selected"));
    QAction *copyAll = menu.addAction(tr("Copy All"));

    copySelected->setEnabled(!m_tree->selectedItems().isEmpty());
    copyAll->setEnabled(m_tree->topLevelItemCount() > 0);

    QAction *chosen = menu.exec(m_tree->viewport()->mapToGlobal(pos));
    if (chosen == copySelected) {
        copySelectedRows();
    } else if (chosen == copyAll) {
        copyAllRows();
    }
}

void ChangeOverviewWidget::copySelectedRows()
{
    const QString text = buildClipboardText(m_tree->selectedItems());
    if (!text.isEmpty()) {
        QApplication::clipboard()->setText(text);
    }
}

void ChangeOverviewWidget::copyAllRows()
{
    QList<QTreeWidgetItem*> items;
    items.reserve(m_tree->topLevelItemCount());

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        items.append(m_tree->topLevelItem(i));
    }

    const QString text = buildClipboardText(items);
    if (!text.isEmpty()) {
        QApplication::clipboard()->setText(text);
    }
}

QString ChangeOverviewWidget::buildClipboardText(const QList<QTreeWidgetItem*>& items) const
{
    QStringList lines;
    lines.reserve(items.size() + 1);
    lines << "Chunk\tField\tOld\tNew";

    for (QTreeWidgetItem *item : items) {
        if (!item) {
            continue;
        }

        const QString chunk = item->text(0);
        const QString field = item->text(1);
        const QString oldValue = item->text(2);
        const QString newValue = item->text(3);

        if (chunk.isEmpty() && field.isEmpty() && oldValue.isEmpty() && newValue.isEmpty()) {
            continue;
        }

        lines << QString("%1\t%2\t%3\t%4").arg(chunk, field, oldValue, newValue);
    }

    return lines.size() > 1 ? lines.join('\n') : QString();
}
