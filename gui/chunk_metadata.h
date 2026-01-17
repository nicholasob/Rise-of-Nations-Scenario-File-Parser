#ifndef CHUNK_METADATA_H
#define CHUNK_METADATA_H

#include <string>
#include <vector>
#include <map>
#include <QColor>
#include "chunk_types.h"

/**
 * @brief Information about a field within a chunk structure
 */
struct FieldInfo {
    std::string name;       // Field name (e.g., "width", "player_name")
    size_t offset;          // Offset within the chunk data (excluding header)
    size_t size;            // Size in bytes
    std::string type;       // Type name (e.g., "uint32_t", "char16_t[32]")
    std::string description; // Optional description

    FieldInfo(const std::string& n, size_t off, size_t sz, const std::string& t, const std::string& desc = "")
        : name(n), offset(off), size(sz), type(t), description(desc) {}
};

/**
 * @brief Metadata about a chunk type
 */
struct ChunkInfo {
    ChunkType type;
    std::string name;
    std::string category;   // e.g., "Map", "Player", "Units", "Advanced Features"
    size_t dataSize;        // Expected data size (0 for variable-length)
    QColor color;           // Highlight color for visualization
    std::vector<FieldInfo> fields;
    bool isContainer;       // True if chunk is just a container with no data

    ChunkInfo()
        : type(ChunkType::SCENARIO_HEADER)
        , dataSize(0)
        , color(Qt::white)
        , isContainer(false) {}

    ChunkInfo(ChunkType t, const std::string& n, const std::string& cat, size_t sz, QColor col, bool container = false)
        : type(t), name(n), category(cat), dataSize(sz), color(col), isContainer(container) {}
};

/**
 * @brief Registry of chunk metadata
 *
 * Provides field-level information for all known chunk types
 */
class ChunkMetadata
{
public:
    static ChunkMetadata& instance();

    // Query methods
    const ChunkInfo* getChunkInfo(ChunkType type) const;
    const FieldInfo* getFieldAtOffset(ChunkType type, size_t offset) const;
    QColor getColorForChunk(ChunkType type) const;
    std::string getCategory(ChunkType type) const;

    // Utility
    bool isRegistered(ChunkType type) const;

private:
    ChunkMetadata();
    ~ChunkMetadata() = default;

    // Non-copyable
    ChunkMetadata(const ChunkMetadata&) = delete;
    ChunkMetadata& operator=(const ChunkMetadata&) = delete;

    void registerChunks();
    void registerChunk(const ChunkInfo& info);

    // Template method for auto-registering reflected chunks
    template<typename T>
    void registerReflectedChunk(ChunkType type, const std::string& name,
                                const std::string& category, size_t size, QColor color);

    // Convert FieldDescriptor to FieldInfo
    static std::vector<FieldInfo> convertDescriptors(const std::vector<struct FieldDescriptor>& descriptors);

    // Category colors
    QColor colorMap;
    QColor colorPlayer;
    QColor colorUnits;
    QColor colorAdvanced;
    QColor colorMetadata;
    QColor colorEditor;
    QColor colorDiplomacy;
    QColor colorResource;
    QColor colorTrigger;
    QColor colorVisibility;
    QColor colorContainer;

    std::map<ChunkType, ChunkInfo> m_chunkRegistry;
};

#endif // CHUNK_METADATA_H
