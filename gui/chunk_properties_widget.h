#ifndef CHUNK_PROPERTIES_WIDGET_H
#define CHUNK_PROPERTIES_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>

class ScenarioDocument;
struct Chunk;

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

private:
    void setupUI();
    void displayHeader(const Chunk* chunk);
    void displayFields(const Chunk* chunk);

    ScenarioDocument *m_document;
    QLabel *m_chunkInfoLabel;
    QTableWidget *m_table;
};

#endif // CHUNK_PROPERTIES_WIDGET_H
