#ifndef CHANGE_OVERVIEW_WIDGET_H
#define CHANGE_OVERVIEW_WIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QPoint>
#include <QString>
#include "scenario_document.h"

/**
 * @brief Presents a list of field-level changes between reloads.
 *
 * Clicking an entry emits highlightRequested(offset, length) so viewers
 * (hex/properties) can jump to the changed bytes.
 */
class ChangeOverviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChangeOverviewWidget(QWidget *parent = nullptr);
    void setChanges(const QVector<FieldChange>& changes);
    void clear();

signals:
    void highlightRequested(qulonglong offset, qulonglong length);

private slots:
    void handleItemActivated(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint& pos);
    void copySelectedRows();
    void copyAllRows();

private:
    QString buildClipboardText(const QList<QTreeWidgetItem*>& items) const;
    QTreeWidget *m_tree;
};

#endif // CHANGE_OVERVIEW_WIDGET_H
