#ifndef SCENARIO_DOCUMENT_H
#define SCENARIO_DOCUMENT_H

#include <QObject>
#include <QString>
#include <vector>
#include <cstddef>
#include <memory>
#include <QDateTime>
#include <QVector>
#include <QPair>
#include <QString>
#include <QMetaType>

#include "data_structures.h"
#include "scenario_editor.h"

class ScenarioModifier;
class QFileSystemWatcher;
class QTimer;

struct FieldChange {
    QString chunkPath;      // hierarchical index path (e.g., "0/2/1")
    QString chunkName;      // human-friendly chunk name
    QString fieldName;      // metadata field name
    QString oldValue;       // previous value (formatted)
    QString newValue;       // new value (formatted)
    qulonglong offset;      // file offset for highlighting
    qulonglong length;      // length in bytes
};

struct ByteChange {
    qulonglong offset;
    int oldValue; // -1 if not present before (added)
    int newValue; // -1 if not present after (removed)
};

Q_DECLARE_METATYPE(FieldChange);
Q_DECLARE_METATYPE(QVector<FieldChange>);
Q_DECLARE_METATYPE(ByteChange);
Q_DECLARE_METATYPE(QVector<ByteChange>);

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
    qulonglong getChunkDataStart(const Chunk& chunk) const;
    void selectChunk(const Chunk* chunk);

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
    void fileReloadDiff(const QVector<QPair<qulonglong, qulonglong>>& ranges);
    void fileReloadFieldChanges(const QVector<FieldChange>& changes);
    void fileReloadByteChanges(const QVector<ByteChange>& changes);

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
    QTimer* m_reloadTimer;
    QString m_pendingReloadPath;
    int m_reloadRetryCount;
    int m_reloadRetryMax;
    int m_reloadRetryDelayMs;
    QString m_lastLoadError;

    // Helper methods
    void clearData();
    const Chunk* findChunkAtOffsetRecursive(size_t offset, const Chunk& chunk) const;
    void updateFileWatcher();
    QVector<QPair<qulonglong, qulonglong>> computeDiffRanges(
        const std::vector<uint8_t>& oldData,
        const std::vector<uint8_t>& newData) const;
    QVector<ByteChange> computeByteChanges(
        const std::vector<uint8_t>& oldData,
        const std::vector<uint8_t>& newData) const;
    void scheduleReloadAttempt(const QString& path);
    void attemptReloadFromWatcher();
    struct FlatChunkInfo {
        const Chunk* chunk;
        QString path; // hierarchical index path
    };
    void flattenChunks(const std::vector<Chunk>& chunks,
                       QVector<FlatChunkInfo>& out,
                       const QString& prefix = QString()) const;
    QVector<FieldChange> computeFieldChanges(
        const std::vector<Chunk>& oldChunks,
        const std::vector<Chunk>& newChunks) const;
    QString formatFieldValue(const FieldInfo& field,
                             const Chunk& chunk) const;
};

#endif // SCENARIO_DOCUMENT_H
