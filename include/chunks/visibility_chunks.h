#pragma once
#include "../base/byte_convertible.h"
#include "../common/common_types.h"
#include <cstdint>

//visibility value interpretation (based on reverse engineering)
struct VisibilityFlags {
    enum : uint16_t {
        UNKNOWN_0       = 0x0001,   //unknown flag
        UNKNOWN_1       = 0x0002,   //unknown flag  
        UNKNOWN_2       = 0x0004,   //unknown flag
        UNKNOWN_3       = 0x0008,   //unknown flag
        UNKNOWN_4       = 0x0010,   //unknown flag
        UNKNOWN_5       = 0x0020,   //unknown flag
        UNKNOWN_6       = 0x0040,   //unknown flag
        UNKNOWN_7       = 0x0080,   //unknown flag
        UNKNOWN_8       = 0x0100,   //unknown flag
        UNKNOWN_9       = 0x0200,   //unknown flag
        UNKNOWN_10      = 0x0400,   //unknown flag
        UNKNOWN_11      = 0x0800,   //unknown flag
        UNKNOWN_12      = 0x1000,   //unknown flag
        UNKNOWN_13      = 0x2000,   //unknown flag
        UNKNOWN_14      = 0x4000,   //unknown flag
        UNKNOWN_15      = 0x8000,   //unknown flag
    };
    
    static std::string decode_flags(uint16_t value) {
        std::string result;
        for (int i = 0; i < 16; i++) {
            if (value & (1 << i)) {
                if (!result.empty()) result += " | ";
                result += "BIT_" + std::to_string(i);
            }
        }
        return result.empty() ? "NONE" : result;
    }
};

#pragma pack(push, 1)
//chunk 0x64 - Triangle Count/Visibility Index Data
//this chunk appears to contain a count that determines how many
//entries will be read from the subsequent chunk 0x65
struct VisibilityTriangleDataHeaderChunk0x64 : public ByteConvertible<VisibilityTriangleDataHeaderChunk0x64> {
    //number of triangles/indices that follow in chunk 0x65
    uint32_t triangle_count;
    
    //the actual triangle data is NOT in this chunk
    //its in the immediately following chunk 0x65 which contains
    //triangle_count * sizeof(uint16_t) bytes of index data
};
#pragma pack(pop)
static_assert(sizeof(VisibilityTriangleDataHeaderChunk0x64) == 0x4, "TriangleDataChunk0x64 must be exactly 4 bytes");

#pragma pack(push, 1)
//chunk 0x65 - Triangle Index Array Data
//this chunk contains a variable-length array of 16-bit triangle indices
//the count is determined by the triangle_count field in the preceding chunk 0x64
struct VisibilityTriangleIndicesChunk0x65 : public VariableLengthArrayChunk<VisibilityTriangleIndicesChunk0x65, uint16_t> {
    std::vector<uint16_t> indices;

    VisibilityTriangleIndicesChunk0x65() = default;

    VisibilityTriangleIndicesChunk0x65(const VisibilityTriangleDataHeaderChunk0x64& header) {
        indices.resize(header.triangle_count);
    }

    VisibilityTriangleIndicesChunk0x65(const size_t& triangle_count) {
        indices.resize(triangle_count);
    }

    //implementation of pure virtual functions
    std::vector<uint16_t>& get_container() override {
        return indices;
    }
    
    const std::vector<uint16_t>& get_container() const override {
        return indices;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
//chunk 0x59: Map Object References
struct TileVisabilityChunk0x65 : public VariableLengthArrayChunk<TileVisabilityChunk0x65, uint16_t> {
    std::vector<uint16_t> tile_visability_entries;

    TileVisabilityChunk0x65() = default;

    TileVisabilityChunk0x65(const size_t& unit_count) {
        tile_visability_entries.resize(unit_count);
    }

    //implementation of pure virtual functions
    std::vector<uint16_t>& get_container() override {
        return tile_visability_entries;
    }
    
    const std::vector<uint16_t>& get_container() const override {
        return tile_visability_entries;
    }
};
#pragma pack(pop)
