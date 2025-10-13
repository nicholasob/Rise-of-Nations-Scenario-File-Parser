#pragma once
#include "../base/byte_convertible.h"
#include "../common/common_types.h"
#include <cstdint>

//enum for tile property entry types based on trial and error
enum class TilePropertyEntryType : uint8_t {
    LAND = 0x00,
    SANDY = 0x01,
    OCEAN = 0x02
};

#pragma pack(push, 1)
struct TilePropertyEntry {
    union {
        /*
            Land (W) (WX: X, WY: Y) evel: E (the editor)
        */
        struct {
            /*
                0x01 = unknown
                0x04 = forest
                0x40 = cliffs
                0x08 = coast
                0x8000 = Coast (not necessarely???? one time not...)
                0x0800 = Land next to coast


            */
            // processed terrain flags
            // based on bit mapping in 0x009a89b1-0x009a8a18:
            uint16_t terrain_flags;      // Processed terrain properties:
                                        // 0x01: Walkable/Passable (source 0x8 -> 0x1)
                                        // 0x04: Elevated/Hill (source 0x20 -> 0x4) 
                                        // 0x08: Resource present (source 0x400 -> 0x8)
                                        // 0x10: Water/Impassable (source 0x800 -> 0x10)
                                        // 0x20: Special terrain (source 0x1000 -> 0x20)
                                        // 0x40: Road/Path (source 0x40 -> 0x40)
                                        // 0x80: Visibility related (source 0x100 -> 0x80)
            
            
            /*
                Range: 0-2

                LAND = 0x00
                SANDY = 0x01 (COAST if terrain_flags set 0x08)
                OCEAN = 0x02
            */
            // Byte 2: extended terrain data 
            TilePropertyEntryType type;   //From source offset +3
            
            /*
                Valid range: 0-4
            */
            // Byte 3: additional terrain information
            uint8_t W;
        };
        uint32_t raw_value;             //Raw 32-bit value for the tile
    };
};
#pragma pack(pop)
static_assert(sizeof(TilePropertyEntry) == 0x4, "TilePropertyEntry must be exactly 4 bytes");

//terrain flag bit definitions for easy reference
namespace TerrainFlags {
    constexpr uint8_t WALKABLE         = 0x01;  // Tile is passable
    constexpr uint8_t ELEVATED         = 0x04;  // Hill/elevated terrain
    constexpr uint8_t RESOURCE         = 0x08;  // Contains resources
    constexpr uint8_t WATER            = 0x10;  // Water/impassable
    constexpr uint8_t SPECIAL          = 0x20;  // Special terrain feature
    constexpr uint8_t ROAD             = 0x40;  // Road/path
    constexpr uint8_t VISIBILITY       = 0x80;  // Visibility-related
}

//source terrain type bit mappings (from PTR_DAT_00c06188 + 0x138)
struct SourceTerrainFlags {
    uint16_t raw_source;
    
    //source bit meanings:
    static constexpr uint16_t TERRAIN_TYPE_MASK = 0x30;  // Bits 4-5
    static constexpr uint16_t TERRAIN_TYPE_SPECIAL = 0x20; // Special terrain when (raw & 0x30) == 0x20
    static constexpr uint16_t RESOURCE_FLAG = 0x400;     // Bit 10: Resource present
    static constexpr uint16_t WATER_FLAG = 0x800;        // Bit 11: Water/Impassable
};

enum class TerrainTypeFlags : uint16_t {
    LAND_TERRAIN = 0x00,
    OCEAN = 0x10,
    BEACH = 0x30,
    RIVER = 0x02
};
static_assert(sizeof(TerrainTypeFlags) == 0x2, "TerrainTypeFlags must be exactly 2 bytes");

#pragma pack(push, 1)
struct MapObjectEntry {
    uint32_t object_id; //Object ID/Reference (4 bytes each)
};
#pragma pack(pop)
static_assert(sizeof(MapObjectEntry) == 0x4, "MapObjectEntry must be exactly 4 bytes");

#pragma pack(push, 1)
struct MapNameChunk0x51 : public ByteConvertible<MapNameChunk0x51> {
    char16_t map_name[100]; //100 wide characters = 200 bytes, null terminated if shorter than 100 chars (hmmmm)
};
#pragma pack(pop)
static_assert(sizeof(MapNameChunk0x51) == 0xC8, "MapNameChunk0x51 must be exactly 200 bytes");

#pragma pack(push, 1)
struct MapStructureChunk0xf : public ByteConvertible<MapStructureChunk0xf> {
    // Chunk Data (12 bytes total) - based on assembly analysis at 0x009a8922-0x009a895b:
    // the values are packed as: total_tiles, map_width, map_height
    uint32_t total_tiles;               // Total number of tiles (width × height)
    uint32_t map_width;                 // Map width in tiles  
    uint32_t map_height;                // Map height in tiles
};
#pragma pack(pop)
static_assert(sizeof(MapStructureChunk0xf) == 0xC, "MapStructureChunk0xf must be exactly 12 bytes");

#pragma pack(push, 1)
struct TilePropertiesChunk0x10 : public VariableLengthArrayChunk<TilePropertiesChunk0x10, TilePropertyEntry> {
    std::vector<TilePropertyEntry> tiles_entries;

    TilePropertiesChunk0x10() = default;

    TilePropertiesChunk0x10(const MapStructureChunk0xf& header) {
        tiles_entries.resize(header.total_tiles);
    }

    TilePropertiesChunk0x10(const size_t& unit_count) {
        tiles_entries.resize(unit_count);
    }

    std::vector<TilePropertyEntry>& get_container() override {
        return tiles_entries;
    }
    
    const std::vector<TilePropertyEntry>& get_container() const override {
        return tiles_entries;
    }
};
#pragma pack(pop)

/*
    Tiles Scaling: scaled_total_tiles = original_tiles × 0x10 (multiply by 16)
    Width Scaling: scaled_width = original_width << 2 (multiply by 4)
    Height Scaling: scaled_height = original_height << 2 (multiply by 4)
*/
#pragma pack(push, 1)
struct TerrainLayoutChunk0x11 : public ByteConvertible<TerrainLayoutChunk0x11> {
    // Chunk Data (12 bytes total)
    union {
        struct {
            uint32_t scaled_total_tiles;    // total_tiles * 16 (iVar17)
            uint32_t scaled_width;          // map_width * 4 (left-shifted by 2)
            uint32_t scaled_height;         // map_height * 4 (left-shifted by 2)
        };
        struct {
            uint64_t first_8_bytes;         // CONCAT44(width_scaled, total_tiles_scaled)
            uint32_t height_scaled;         // height * 4
        };
    };
};
#pragma pack(pop)
static_assert(sizeof(TerrainLayoutChunk0x11) == 0xC, "TerrainLayoutChunk0x11 must be exactly 12 bytes");

/*
Terrain:
	0x00 = Land
	0x10 = Ocean
	0x30 = Beach?
	0x20 = River
*/
#pragma pack(push, 1)
//chunk 0x12: Terrain Type Data
struct TerrainTypeChunk0x12 : public VariableLengthArrayChunk<TerrainTypeChunk0x12, TerrainTypeFlags> {
    // variable length data: total_tiles * 16 entries of TerrainTypeFlags
    // each entry is 2 bytes (uint16_t)
    std::vector<TerrainTypeFlags> terrain_entries;

    TerrainTypeChunk0x12() = default;

    TerrainTypeChunk0x12(const TerrainLayoutChunk0x11& header) {
        terrain_entries.resize(header.scaled_total_tiles);
    }

    TerrainTypeChunk0x12(const size_t& unit_count) {
        terrain_entries.resize(unit_count);
    }

    std::vector<TerrainTypeFlags>& get_container() override {
        return terrain_entries;
    }
    
    const std::vector<TerrainTypeFlags>& get_container() const override {
        return terrain_entries;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
// Chunk 0x59: Map Object References
struct MapObjectChunk0x59 : public VariableLengthArrayChunk<MapObjectChunk0x59, MapObjectEntry> {
    std::vector<MapObjectEntry> map_object_entries;

    MapObjectChunk0x59() = default;

    MapObjectChunk0x59(const size_t& unit_count) {
        map_object_entries.resize(unit_count);
    }

    std::vector<MapObjectEntry>& get_container() override {
        return map_object_entries;
    }
    
    const std::vector<MapObjectEntry>& get_container() const override {
        return map_object_entries;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MapMetadataChunk0x69 : public ByteConvertible<MapMetadataChunk0x69> {
    uint32_t tile_count;
};
#pragma pack(pop)
static_assert(sizeof(MapMetadataChunk0x69) == 0x4, "MapMetadataChunk0x69 must be exactly 4 bytes");

#pragma pack(push, 1)
// Chunk 0x6a - dont know really what this represents TODO: need to test
struct ExtendedMapData0x6a : public VariableLengthArrayChunk<ExtendedMapData0x6a, uint8_t> {
    std::vector<uint8_t> extended_map_data;

    ExtendedMapData0x6a() = default;

    ExtendedMapData0x6a(const size_t& unit_count) {
        extended_map_data.resize(unit_count);
    }

    std::vector<uint8_t>& get_container() override {
        return extended_map_data;
    }
    
    const std::vector<uint8_t>& get_container() const override {
        return extended_map_data;
    }
};
#pragma pack(pop)
