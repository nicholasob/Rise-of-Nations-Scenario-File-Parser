#include "chunk_tree_widget.h"
#include "scenario_document.h"
#include "chunk_types.h"
#include "chunk_metadata.h"
#include <QVBoxLayout>
#include <QHeaderView>

ChunkTreeWidget::ChunkTreeWidget(ScenarioDocument *document, QWidget *parent)
    : QWidget(parent)
    , m_document(document)
{
    setupUI();

    connect(m_document, &ScenarioDocument::dataLoaded,
            this, &ChunkTreeWidget::onDataLoaded);
}

void ChunkTreeWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderLabels({"Chunk Type", "Offset", "Size", "Children"});
    m_tree->setColumnWidth(0, 250);
    m_tree->setColumnWidth(1, 100);
    m_tree->setColumnWidth(2, 80);
    m_tree->setColumnWidth(3, 80);
    m_tree->setAlternatingRowColors(true);

    connect(m_tree, &QTreeWidget::itemClicked,
            this, &ChunkTreeWidget::onItemClicked);

    layout->addWidget(m_tree);
}

void ChunkTreeWidget::refresh()
{
    buildTree();
}

void ChunkTreeWidget::onDataLoaded()
{
    buildTree();
}

void ChunkTreeWidget::buildTree()
{
    m_tree->clear();

    const auto& chunks = m_document->getChunks();
    for (const auto& chunk : chunks) {
        addChunkToTree(chunk, nullptr);
    }

    m_tree->expandToDepth(1); // Expand first two levels
}

void ChunkTreeWidget::addChunkToTree(const Chunk& chunk, QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();

    // Column 0: Chunk type name
    auto& metadata = ChunkMetadata::instance();
    const auto* chunkInfo = metadata.getChunkInfo(chunk.header.chunk_type_identifier);
    std::string typeName = chunkInfo ? chunkInfo->name : ("UNKNOWN_" + std::to_string(static_cast<uint16_t>(chunk.header.chunk_type_identifier)));
    item->setText(0, QString::fromStdString(typeName));

    // Column 1: Offset
    item->setText(1, QString("0x%1").arg(chunk.file_offset, 0, 16).toUpper());

    // Column 2: Size
    item->setText(2, QString::number(chunk.header.chunk_size));

    // Column 3: Children count
    item->setText(3, QString::number(chunk.children.size()));

    // Store chunk offset as data
    item->setData(0, Qt::UserRole, QVariant::fromValue(chunk.file_offset));

    // Set color based on chunk metadata
    QColor color = metadata.getColorForChunk(chunk.header.chunk_type_identifier);
    item->setBackground(0, QBrush(color));

    if (parent) {
        parent->addChild(item);
    } else {
        m_tree->addTopLevelItem(item);
    }

    // Recursively add children
    for (const auto& child : chunk.children) {
        addChunkToTree(child, item);
    }
}

void ChunkTreeWidget::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (!item) {
        return;
    }

    size_t offset = item->data(0, Qt::UserRole).value<size_t>();
    const Chunk* chunk = m_document->findChunkAtOffset(offset);

    if (chunk) {
        emit chunkSelected(chunk);
    }
}

void ChunkTreeWidget::highlightChunk(const Chunk* chunk)
{
    if (!chunk) {
        return;
    }
    QTreeWidgetItem* item = findItemByOffset(chunk->file_offset);
    if (item) {
        m_tree->setCurrentItem(item);
        m_tree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }
}

QTreeWidgetItem* ChunkTreeWidget::findItemByOffset(size_t offset, QTreeWidgetItem *parent) const
{
    const int childCount = parent ? parent->childCount() : m_tree->topLevelItemCount();
    for (int i = 0; i < childCount; ++i) {
        QTreeWidgetItem* item = parent ? parent->child(i) : m_tree->topLevelItem(i);
        if (!item) continue;
        size_t itemOffset = item->data(0, Qt::UserRole).value<size_t>();
        if (itemOffset == offset) {
            return item;
        }
        if (item->childCount() > 0) {
            if (QTreeWidgetItem* found = findItemByOffset(offset, item)) {
                return found;
            }
        }
    }
    return nullptr;
}
