#ifndef CHUNK_PROPERTIES_WIDGET_H
#define CHUNK_PROPERTIES_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVector>
#include <QPair>
#include <limits>

class ScenarioDocument;
struct Chunk;
struct FieldChange;

/**
 * @brief Widget displaying decoded chunk properties
 */
class ChunkPropertiesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChunkPropertiesWidget(ScenarioDocument *document, QWidget *parent = nullptr);
    ~ChunkPropertiesWidget() = default;

public slots:
    void displayChunk(const Chunk* chunk);
    void clear();
    void setFieldChanges(const QVector<FieldChange>& changes);
    void highlightFieldAtOffset(qulonglong offset);

signals:
    void fieldRangeSelected(qulonglong offset, qulonglong length);

private:
    void setupUI();
    void displayHeader(const Chunk* chunk);
    void displayFields(const Chunk* chunk);
    bool isFieldChanged(const Chunk* chunk, size_t absoluteOffset, size_t size, QString& oldVal, QString& newVal) const;
    void handleSelectionChanged();

    ScenarioDocument *m_document;
    QLabel *m_chunkInfoLabel;
    QTableWidget *m_table;
    QVector<FieldChange> m_fieldChanges;
    QVector<QPair<qulonglong, qulonglong>> m_rowRanges;
    const Chunk* m_currentChunk;
    qulonglong m_pendingHighlightOffset = std::numeric_limits<qulonglong>::max();
};

#endif // CHUNK_PROPERTIES_WIDGET_H
