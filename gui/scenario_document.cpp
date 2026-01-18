#include "scenario_document.h"
#include "scenario_modifier.h"
#include "chunk_metadata.h"
#include "compression.h"
#include "chunk_serializer.h"
#include "chunk_validator.h"
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QVector>
#include <QPair>
#include <QTimer>
#include <QStringList>
#include <algorithm>
#include <fstream>
#include <stdexcept>

ScenarioDocument::ScenarioDocument(QObject *parent)
    : QObject(parent)
    , m_dirty(false)
    , m_editor(nullptr)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_autoReloadEnabled(true)
    , m_reloadTimer(new QTimer(this))
    , m_reloadRetryCount(0)
    , m_reloadRetryMax(5)
    , m_reloadRetryDelayMs(200)
{
    qRegisterMetaType<FieldChange>("FieldChange");
    qRegisterMetaType<QVector<FieldChange>>("QVector<FieldChange>");
    qRegisterMetaType<ByteChange>("ByteChange");
    qRegisterMetaType<QVector<ByteChange>>("QVector<ByteChange>");
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ScenarioDocument::handleFileChanged);
    m_reloadTimer->setSingleShot(true);
    connect(m_reloadTimer, &QTimer::timeout,
            this, &ScenarioDocument::attemptReloadFromWatcher);
}

bool ScenarioDocument::loadFile(const QString& filePath)
{
    m_lastLoadError.clear();
    try {
        // Load into temporaries to avoid losing current state on failure
        auto newEditor = std::make_unique<ScenarioEditor>();
        if (!newEditor->LoadScenario(filePath.toStdString())) {
            const std::string& detail = newEditor->GetLastError();
            if (!detail.empty()) {
                m_lastLoadError = QString::fromStdString(detail);
                emit errorOccurred(QString("Failed to parse scenario data: %1").arg(m_lastLoadError));
            } else {
                emit errorOccurred("Failed to parse scenario data");
            }
            return false;
        }

        // Get chunks from editor
        std::vector<Chunk> newChunks = std::vector<Chunk>(newEditor->getChunks());

        // Get decompressed data - re-serialize for hex view
        ChunkSerializer serializer;
        std::vector<uint8_t> newData = serializer.SerializeChunks(newChunks);

        // Commit new state
        m_editor = std::move(newEditor);
        m_chunks = std::move(newChunks);
        m_data = std::move(newData);
        m_filePath = filePath;
        m_dirty = false;
        updateFileWatcher();

        emit dataLoaded();
        return true;

    } catch (const std::exception& e) {
        m_lastLoadError = QString::fromUtf8(e.what());
        emit errorOccurred(QString("Error loading file: %1").arg(e.what()));
        return false;
    }
}

bool ScenarioDocument::saveFile(const QString& filePath, bool compress)
{
    // TODO: Implement save functionality
    // Temporarily disabled for minimal build
    Q_UNUSED(filePath);
    Q_UNUSED(compress);

    emit errorOccurred("Save functionality not yet implemented in minimal build");
    return false;

    /* COMMENTED OUT FOR MINIMAL BUILD - Re-enable once APIs are verified
    try {
        // Validate chunks before saving
        ChunkValidator validator;
        auto validationResult = validator.ValidateChunks(m_chunks);

        if (!validationResult.errors.empty()) {
            QString errorMsg = "Validation errors:\n";
            for (const auto& error : validationResult.errors) {
                errorMsg += QString::fromStdString(error) + "\n";
            }
            emit errorOccurred(errorMsg);
            return false;
        }

        // Serialize chunks to binary
        ChunkSerializer serializer;
        std::vector<uint8_t> serializedData = serializer.SerializeChunks(m_chunks);

        // Compress if requested
        std::vector<uint8_t> outputData;
        if (compress) {
            std::vector<uint8_t> tempOut;
            if (!GzipHelper::compressGzip(serializedData, tempOut)) {
                emit errorOccurred("Failed to compress data");
                return false;
            }
            outputData = std::move(tempOut);
        } else {
            outputData = std::move(serializedData);
        }

        // Write to file
        std::ofstream outFile(filePath.toStdString(), std::ios::binary);
        if (!outFile.is_open()) {
            emit errorOccurred("Failed to create output file: " + filePath);
            return false;
        }

        outFile.write(reinterpret_cast<const char*>(outputData.data()), outputData.size());
        outFile.close();

        // Update state
        m_filePath = filePath;
        m_dirty = false;
        emit dirtyChanged(false);

        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error saving file: %1").arg(e.what()));
        return false;
    }
    */
}

uint8_t ScenarioDocument::getByteAt(size_t offset) const
{
    if (offset >= m_data.size()) {
        throw std::out_of_range("Offset out of range");
    }
    return m_data[offset];
}

void ScenarioDocument::setByteAt(size_t offset, uint8_t value)
{
    // TODO: Implement byte editing
    // Temporarily disabled for minimal read-only build
    Q_UNUSED(offset);
    Q_UNUSED(value);

    // Silently ignore edits in read-only mode
    return;

    /* COMMENTED OUT FOR MINIMAL BUILD
    if (offset >= m_data.size()) {
        throw std::out_of_range("Offset out of range");
    }

    if (m_data[offset] != value) {
        m_data[offset] = value;
        setDirty(true);
        emit dataChanged(offset, 1);
    }
    */
}

const Chunk* ScenarioDocument::findChunkAtOffset(size_t offset) const
{
    for (const auto& chunk : m_chunks) {
        if (chunk.file_offset == offset) {
            return &chunk;
        }
        const Chunk* found = findChunkAtOffsetRecursive(offset, chunk);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

const Chunk* ScenarioDocument::findChunkContainingOffset(size_t offset, const Chunk* parent) const
{
    const std::vector<Chunk>& chunks = parent ? parent->children : m_chunks;

    for (const auto& chunk : chunks) {
        size_t chunkStart = chunk.file_offset;
        size_t chunkEnd = chunkStart + chunk.header.chunk_size;

        if (offset >= chunkStart && offset < chunkEnd) {
            // Check children first for more specific match
            const Chunk* childMatch = findChunkContainingOffset(offset, &chunk);
            if (childMatch) {
                return childMatch;
            }
            return &chunk;
        }
    }

    return nullptr;
}

void ScenarioDocument::setDirty(bool dirty)
{
    if (m_dirty != dirty) {
        m_dirty = dirty;
        emit dirtyChanged(dirty);
    }
}

void ScenarioDocument::refreshFromEditor()
{
    if (m_editor && m_editor->IsLoaded()) {
        m_chunks = std::vector<Chunk>(m_editor->getChunks());

        // Re-serialize to get updated binary data
        ChunkSerializer serializer;
        m_data = serializer.SerializeChunks(m_chunks);

        setDirty(true);
        emit dataLoaded(); // Trigger full refresh
    }
}

void ScenarioDocument::clearData()
{
    m_data.clear();
    m_chunks.clear();
    if (m_fileWatcher) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }
    m_lastModifiedTime = QDateTime();
    m_filePath.clear();
    m_dirty = false;
    m_editor.reset();
}

const Chunk* ScenarioDocument::findChunkAtOffsetRecursive(size_t offset, const Chunk& chunk) const
{
    if (chunk.file_offset == offset) {
        return &chunk;
    }

    for (const auto& child : chunk.children) {
        if (child.file_offset == offset) {
            return &child;
        }
        const Chunk* found = findChunkAtOffsetRecursive(offset, child);
        if (found) {
            return found;
        }
    }

    return nullptr;
}

qulonglong ScenarioDocument::getChunkDataStart(const Chunk& chunk) const
{
    // Data starts after header + children; chunk.header.chunk_size includes everything
    size_t dataSize = chunk.data.size();
    return static_cast<qulonglong>(chunk.file_offset + (chunk.header.chunk_size - dataSize));
}

void ScenarioDocument::selectChunk(const Chunk* chunk)
{
    if (chunk) {
        emit chunkSelected(chunk);
    }
}

void ScenarioDocument::handleFileChanged(const QString& path)
{
    if (!m_autoReloadEnabled || m_dirty) {
        return;
    }

    scheduleReloadAttempt(path);
}

void ScenarioDocument::scheduleReloadAttempt(const QString& path)
{
    m_pendingReloadPath = path;
    m_reloadRetryCount = 0;
    m_reloadTimer->start(m_reloadRetryDelayMs);
}

void ScenarioDocument::attemptReloadFromWatcher()
{
    if (m_pendingReloadPath.isEmpty()) {
        return;
    }

    const QString path = m_pendingReloadPath;

    QFileInfo info(path);
    if (!info.exists()) {
        return;
    }

    const auto modified = info.lastModified();
    if (modified == m_lastModifiedTime) {
        return; // Ignore duplicate notifications for the same timestamp
    }

    // Persist timestamp before reload to prevent recursive triggers
    m_lastModifiedTime = modified;

    const auto previousData = m_data; // Snapshot before reload
    const auto previousChunks = m_chunks;

    if (loadFile(path)) {
        emit fileReloaded(path);
        emit fileReloadDiff(computeDiffRanges(previousData, m_data));
        emit fileReloadFieldChanges(computeFieldChanges(previousChunks, m_chunks));
        emit fileReloadByteChanges(computeByteChanges(previousData, m_data));
        m_pendingReloadPath.clear();
        return;
    }

    ++m_reloadRetryCount;
    if (m_reloadRetryCount < m_reloadRetryMax) {
        m_reloadTimer->start(m_reloadRetryDelayMs);
    } else {
        QString msg = QString("Failed to reload %1 after %2 attempts")
                          .arg(QFileInfo(path).fileName())
                          .arg(m_reloadRetryCount);
        if (!m_lastLoadError.isEmpty()) {
            msg += QString(": %1").arg(m_lastLoadError);
        }
        emit errorOccurred(msg);
        m_pendingReloadPath.clear();
    }
}

void ScenarioDocument::updateFileWatcher()
{
    if (!m_fileWatcher) {
        return;
    }

    m_fileWatcher->removePaths(m_fileWatcher->files());

    if (m_filePath.isEmpty()) {
        return;
    }

    QFileInfo info(m_filePath);
    if (!info.exists()) {
        return;
    }

    m_lastModifiedTime = info.lastModified();
    m_fileWatcher->addPath(info.absoluteFilePath());
}

QVector<QPair<qulonglong, qulonglong>> ScenarioDocument::computeDiffRanges(
    const std::vector<uint8_t>& oldData,
    const std::vector<uint8_t>& newData) const
{
    QVector<QPair<qulonglong, qulonglong>> ranges;

    const size_t maxSize = std::max(oldData.size(), newData.size());
    size_t idx = 0;

    while (idx < maxSize) {
        const bool oldValid = idx < oldData.size();
        const bool newValid = idx < newData.size();
        const bool equal = oldValid && newValid && oldData[idx] == newData[idx];

        if (oldValid && newValid && equal) {
            ++idx;
            continue;
        }

        // Start of a diff block
        const size_t start = idx;
        while (idx < maxSize) {
            const bool oValid = idx < oldData.size();
            const bool nValid = idx < newData.size();
            const bool eq = oValid && nValid && oldData[idx] == newData[idx];
            if (oValid && nValid && eq) {
                break;
            }
            ++idx;
        }
        const size_t length = idx - start;
        ranges.push_back(qMakePair(static_cast<qulonglong>(start),
                                   static_cast<qulonglong>(length)));
    }

    return ranges;
}

QVector<ByteChange> ScenarioDocument::computeByteChanges(
    const std::vector<uint8_t>& oldData,
    const std::vector<uint8_t>& newData) const
{
    QVector<ByteChange> changes;
    const size_t maxSize = std::max(oldData.size(), newData.size());
    for (size_t i = 0; i < maxSize; ++i) {
        const bool oldValid = i < oldData.size();
        const bool newValid = i < newData.size();
        const int oldVal = oldValid ? static_cast<int>(oldData[i]) : -1;
        const int newVal = newValid ? static_cast<int>(newData[i]) : -1;
        if (!oldValid || !newValid || oldVal != newVal) {
            changes.push_back(ByteChange{
                static_cast<qulonglong>(i),
                oldVal,
                newVal
            });
        }
    }
    return changes;
}

void ScenarioDocument::flattenChunks(const std::vector<Chunk>& chunks,
                                     QVector<FlatChunkInfo>& out,
                                     const QString& prefix) const
{
    for (size_t i = 0; i < chunks.size(); ++i) {
        const Chunk& chunk = chunks[i];
        QString path = prefix.isEmpty()
            ? QString::number(i)
            : prefix + "/" + QString::number(i);
        out.push_back({&chunk, path});
        if (!chunk.children.empty()) {
            flattenChunks(chunk.children, out, path);
        }
    }
}

QString ScenarioDocument::formatFieldValue(const FieldInfo& field,
                                           const Chunk& chunk) const
{
    if (field.offset + field.size > chunk.data.size()) {
        return "<out of range>";
    }

    QStringList parts;
    for (size_t i = 0; i < field.size; ++i) {
        const auto b = static_cast<uint8_t>(chunk.data[field.offset + i]);
        parts << QString("%1").arg(b, 2, 16, QLatin1Char('0')).toUpper();
    }
    QString hex = parts.join(" ");
    if (field.size > 16) {
        hex = hex.left(16 * 3) + "...";
    }
    return hex;
}

QVector<FieldChange> ScenarioDocument::computeFieldChanges(
    const std::vector<Chunk>& oldChunks,
    const std::vector<Chunk>& newChunks) const
{
    QVector<FieldChange> changes;
    QVector<FlatChunkInfo> oldFlat;
    QVector<FlatChunkInfo> newFlat;
    flattenChunks(oldChunks, oldFlat);
    flattenChunks(newChunks, newFlat);

    const int commonCount = std::min(oldFlat.size(), newFlat.size());
    ChunkMetadata& metadata = ChunkMetadata::instance();

    auto chunkNameFor = [&metadata](ChunkType type) {
        if (const ChunkInfo* info = metadata.getChunkInfo(type)) {
            return QString::fromStdString(info->name);
        }
        return QString("Chunk 0x%1").arg(static_cast<uint16_t>(type), 0, 16).toUpper();
    };

    for (int i = 0; i < commonCount; ++i) {
        const Chunk* oldC = oldFlat[i].chunk;
        const Chunk* newC = newFlat[i].chunk;

        if (oldC->header.chunk_type_identifier != newC->header.chunk_type_identifier) {
            FieldChange fc;
            fc.chunkPath = newFlat[i].path;
            fc.chunkName = chunkNameFor(newC->header.chunk_type_identifier);
            fc.fieldName = "Chunk type";
            fc.oldValue = chunkNameFor(oldC->header.chunk_type_identifier);
            fc.newValue = chunkNameFor(newC->header.chunk_type_identifier);
            fc.offset = newC->file_offset;
            fc.length = newC->header.chunk_size;
            changes.push_back(std::move(fc));
            continue;
        }

        const ChunkInfo* info = metadata.getChunkInfo(newC->header.chunk_type_identifier);
        if (!info) {
            continue;
        }

        for (const auto& field : info->fields) {
            if (field.offset + field.size > oldC->data.size() ||
                field.offset + field.size > newC->data.size()) {
                continue;
            }

            bool differs = false;
            for (size_t b = 0; b < field.size; ++b) {
                if (static_cast<uint8_t>(oldC->data[field.offset + b]) !=
                    static_cast<uint8_t>(newC->data[field.offset + b])) {
                    differs = true;
                    break;
                }
            }

            if (!differs) {
                continue;
            }

            FieldChange fc;
            fc.chunkPath = newFlat[i].path;
            fc.chunkName = QString::fromStdString(info->name);
            fc.fieldName = QString::fromStdString(field.name);
            fc.oldValue = formatFieldValue(field, *oldC);
            fc.newValue = formatFieldValue(field, *newC);
            const qulonglong dataStart = getChunkDataStart(*newC);
            fc.offset = dataStart + static_cast<qulonglong>(field.offset);
            fc.length = static_cast<qulonglong>(field.size);
            changes.push_back(std::move(fc));
        }
    }

    // Added chunks
    for (int i = commonCount; i < newFlat.size(); ++i) {
        const Chunk* c = newFlat[i].chunk;
        FieldChange fc;
        fc.chunkPath = newFlat[i].path;
        fc.chunkName = chunkNameFor(c->header.chunk_type_identifier);
        fc.fieldName = "Chunk added";
        fc.oldValue = "";
        fc.newValue = QString("size %1").arg(c->header.chunk_size);
        fc.offset = c->file_offset;
        fc.length = c->header.chunk_size;
        changes.push_back(std::move(fc));
    }

    // Removed chunks
    for (int i = commonCount; i < oldFlat.size(); ++i) {
        const Chunk* c = oldFlat[i].chunk;
        FieldChange fc;
        fc.chunkPath = oldFlat[i].path;
        fc.chunkName = chunkNameFor(c->header.chunk_type_identifier);
        fc.fieldName = "Chunk removed";
        fc.oldValue = QString("size %1").arg(c->header.chunk_size);
        fc.newValue = "";
        fc.offset = c->file_offset;
        fc.length = c->header.chunk_size;
        changes.push_back(std::move(fc));
    }

    return changes;
}
