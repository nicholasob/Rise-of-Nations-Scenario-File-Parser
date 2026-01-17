#include "scenario_document.h"
#include "scenario_modifier.h"
#include "compression.h"
#include "chunk_serializer.h"
#include "chunk_validator.h"
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <fstream>
#include <stdexcept>

ScenarioDocument::ScenarioDocument(QObject *parent)
    : QObject(parent)
    , m_dirty(false)
    , m_editor(nullptr)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_autoReloadEnabled(true)
{
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ScenarioDocument::handleFileChanged);
}

bool ScenarioDocument::loadFile(const QString& filePath)
{
    try {
        clearData();

        // Create scenario editor and load from file path
        m_editor = std::make_unique<ScenarioEditor>();
        if (!m_editor->LoadScenario(filePath.toStdString())) {
            emit errorOccurred("Failed to parse scenario data");
            clearData();
            return false;
        }

        // Get chunks from editor
        m_chunks = std::vector<Chunk>(m_editor->getChunks());

        // Get decompressed data - re-serialize for hex view
        ChunkSerializer serializer;
        m_data = serializer.SerializeChunks(m_chunks);

        // Store file path
        m_filePath = filePath;
        m_dirty = false;
        updateFileWatcher();

        emit dataLoaded();
        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error loading file: %1").arg(e.what()));
        clearData();
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

void ScenarioDocument::handleFileChanged(const QString& path)
{
    if (!m_autoReloadEnabled || m_dirty) {
        return;
    }

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

    if (loadFile(path)) {
        emit fileReloaded(path);
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
