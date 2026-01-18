#ifndef CHUNK_TREE_WIDGET_H
#define CHUNK_TREE_WIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include "data_structures.h"

class ScenarioDocument;

/**
 * @brief Widget displaying chunk hierarchy as a tree
 */
class ChunkTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChunkTreeWidget(ScenarioDocument *document, QWidget *parent = nullptr);
    ~ChunkTreeWidget() = default;

    void refresh();

signals:
    void chunkSelected(const Chunk* chunk);

public slots:
    void highlightChunk(const Chunk* chunk);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onDataLoaded();

private:
    void setupUI();
    void buildTree();
    void addChunkToTree(const Chunk& chunk, QTreeWidgetItem *parent = nullptr);
    QTreeWidgetItem* findItemByOffset(size_t offset, QTreeWidgetItem *parent = nullptr) const;

    ScenarioDocument *m_document;
    QTreeWidget *m_tree;
};

#endif // CHUNK_TREE_WIDGET_H
