#ifndef SCENARIO_DOCUMENT_H
#define SCENARIO_DOCUMENT_H

#include <QObject>
#include <QString>
#include <vector>
#include <cstddef>
#include <memory>
#include <QDateTime>

#include "data_structures.h"
#include "scenario_editor.h"

class ScenarioModifier;
class QFileSystemWatcher;

/**
 * @brief Central data model for scenario file
 *
 * This class manages the loaded scenario data including:
 * - Original file path
 * - Decompressed binary data
 * - Parsed chunk tree
 * - Modification tracking
 */
class ScenarioDocument : public QObject
{
    Q_OBJECT

public:
    explicit ScenarioDocument(QObject *parent = nullptr);
    ~ScenarioDocument() = default;

    // File operations
    bool loadFile(const QString& filePath);
    bool saveFile(const QString& filePath, bool compress = true);

    // Data access
    const std::vector<uint8_t>& getData() const { return m_data; }
    const std::vector<Chunk>& getChunks() const { return m_chunks; }
    QString getFilePath() const { return m_filePath; }
    size_t getDataSize() const { return m_data.size(); }

    // Byte-level access
    uint8_t getByteAt(size_t offset) const;
    void setByteAt(size_t offset, uint8_t value);

    // Chunk queries
    const Chunk* findChunkAtOffset(size_t offset) const;
    const Chunk* findChunkContainingOffset(size_t offset, const Chunk* parent = nullptr) const;

    // Modification tracking
    bool isDirty() const { return m_dirty; }
    void setDirty(bool dirty);

    // High-level editor
    ScenarioEditor* getEditor() { return m_editor.get(); }
    bool autoReloadEnabled() const { return m_autoReloadEnabled; }
    void setAutoReloadEnabled(bool enabled) { m_autoReloadEnabled = enabled; }

    // Refresh data after modifications
    void refreshFromEditor();

signals:
    void dataLoaded();
    void dataChanged(size_t offset, size_t length);
    void chunkSelected(const Chunk* chunk);
    void dirtyChanged(bool dirty);
    void errorOccurred(const QString& message);
    void fileReloaded(const QString& filePath);

private slots:
    void handleFileChanged(const QString& path);

private:
    QString m_filePath;
    std::vector<uint8_t> m_data;
    std::vector<Chunk> m_chunks;
    bool m_dirty;
    std::unique_ptr<ScenarioEditor> m_editor;
    QFileSystemWatcher* m_fileWatcher;
    QDateTime m_lastModifiedTime;
    bool m_autoReloadEnabled;

    // Helper methods
    void clearData();
    const Chunk* findChunkAtOffsetRecursive(size_t offset, const Chunk& chunk) const;
    void updateFileWatcher();
};

#endif // SCENARIO_DOCUMENT_H
