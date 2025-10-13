#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

/*
38
3a
3b
3c
36
37
2f
30
31
32
33
1b
1e
55
21
23
19
53
1a
54
17
4
18
15
16
24
6b
26
27
56
4c
4d
4e
2c
2d
2e
29
2a
2b
61
62
5d
5e
8
46
47
4a
4b
51
58
f
10
44
11
12
59
5a
5b
63
64
65
68
69
6a
45
5c
60
49
0
6c
b
2
a
3
7
5
6
9
*/
enum class ChunkType : uint16_t {
    // Core structure
    SCENARIO_HEADER                         = 0x00, // root container for the scenario file
    METADATA                                = 0x6C, // final XML/localization metadata (expected 384 bytes in size)

    //map related chunks/features
    MAP_DATA                                = 0x02, // container for all map information
    MAP_STRUCTURE                           = 0x0F, // map dimensions (12 bytes)
    TILE_PROPERTIES                         = 0x10, // per-tile terrain data (4 bytes/tile)
    TERRAIN_LAYOUT_DATA                     = 0x11, // scaled map dimensions (12 bytes)
    TERRAIN_TYPE                            = 0x12, // terrain type flags (2 bytes per position)
    MAP_STRUCTURE_HEADER                    = 0x58, // container for map structure
    TERRAIN_LAYOUT_HEADER                   = 0x44, // container for terrain layout
    MAP_NAME_DATA                           = 0x51, // map name/description (200 bytes)
    MAP_OBJECTS                             = 0x59, // object IDs on map (4 bytes per object)
    OBJECT_GROUPS                           = 0x5A, // grouped object data (variable)
    MAP_LINKS                               = 0x5B, // map connections (2 bytes/link)
    VISIBILITY_HEADER                       = 0x63, // container for visibility data
    VISIBILITY_DATA_COUNT                   = 0x64, // visibility count (4 bytes)
    TILE_VISIBILITY                         = 0x65, // per-tile visibility (2 bytes/tile)
    MAP_METADATA_HEADER                     = 0x68, // container for extended metadata
    MAP_METADATA_DATA                       = 0x69, // metadata count (4 bytes)
    EXTENDED_MAP_DATA                       = 0x6A, // custom map data (variable)

    //advanced map related chunks/features
    ADV_FEATURE_1                           = 0x45, // feature system container
    FEATURE_COUNT                           = 0x46, // number of features (4 bytes)
    FEATURE_DATA                            = 0x47, // feature entries (12 bytes each)
    ADV_FEATURE_4                           = 0x49, // location system container
    LOCATION_COUNT                          = 0x4A, // number of locations (4 bytes)
    LOCATION_DATA                           = 0x4B, // location coords (12 bytes each)
    ADV_FEATURE_2                           = 0x5C, // coordinate group container
    SPATIAL_GROUP_COUNT                     = 0x5D, // number of groups (4 bytes)
    GROUP_COORDINATE_COUNT                  = 0x5E, // subgroup count per group (4 bytes)
    COORDINATE_DATA                         = 0x5F, // coordinate pairs (8 bytes each)
    ADV_FEATURE_3                           = 0x60, // entity system container
    ENTITY_COUNT                            = 0x61, // number of entities (4 bytes)
    ENTITY_DATA                             = 0x62, // entity records (16 bytes each)

    //trigger related chunks/features
    TRIGGER_DATA                            = 0x03, // container for triggers
    TRIGGER_COUNT                           = 0x15, // number of triggers (4 bytes)
    TRIGGER_ENTRIES                         = 0x16, // trigger data (520 bytes each)

    //goods related chunks/features
    GOODS_DATA                              = 0x04, // container for goods
    GOODS_COUNT                             = 0x17, // number of goods (4 bytes)
    GOODS_ENTRIES                           = 0x18, // goods data (520 bytes each)

    //formation system
    FORMATION_DATA                          = 0x05, // container for formations
    FORMATION_HEADER                        = 0x19, // military formation container
    FORMATION_PROPERTIES                    = 0x53, // formation data (580 bytes)
    BUILDING_GROUP_HEADER                   = 0x1A, // building group container - NON CITIES AS I UNDERSTAND IT
    BUILDING_GROUP_PROPERTIES               = 0x54, // building group data (548 bytes)

    //garrison system (shared)
    GARRISON_HEADER                         = 0x55, // garrison container
    GARRISON_COUNT                          = 0x21, // number of garrison units (8 bytes)
    GARRISON_UNIT_NAMES                     = 0x23, // unit names (512 bytes each)

    //editor
    EDITOR_DATA                             = 0x06, // Editor-specific metadata
    EDITOR_PLAYER_NATION_DATA               = 0x2f,
    EDITOR_MAP_TERRAIN_DATA                 = 0x30,
    EDITOR_UNIT_PLACEMENT_STATE             = 0x31,
    EDITOR_RESOURCE_DATA                    = 0x32,
    EDITOR_RESEARCH_PROGRESS                = 0x33,

    //actual Buildings placed on the map (map-level)
    CITY_BUILDING_CONTAINER                 = 0x07, // container for map buildings
    CITY_BUILDING_HEADER                    = 0x1B, // individual building container
    CITY_BUILDING_PROPERTIES                = 0x1E, // building stats (752 bytes)

    //diplomacy
    DIPLOMACY_DATA                          = 0x09, // container for diplomacy data
    DIPLOMATIC_COUNT                        = 0x36, // number of relations (4 bytes)
    DIPLOMATIC_ENTRIES                      = 0x37, // relation entries (520 bytes each)

    //game objects
    OBJECT_DATA                             = 0x0A, // object container
    OBJECT_HEADER                           = 0x38, // object section start
    UNIT_OBJECT                             = 0x3A, // unit object (788 bytes)
    STRUCTURE_OBJECT                        = 0x3B, // building object (820 bytes)
    RESOURCE_OBJECT                         = 0x3C, // resource/special object (760 bytes)

    //player data
    PLAYER_DATA                             = 0x0B, // player system container
    PLAYER_DATA_START                       = 0x24, // start of individual player info
    PLAYER_STATUS_HEADER                    = 0x29, // player status container
    ACTIVE_PLAYER_COUNT                     = 0x2A, // active player count (4 bytes)
    PLAYER_DETAILS                          = 0x2B, // player info (12 bytes/player)
    PLAYER_PROPERTIES                       = 0x6B, // config data (236 bytes)

    //naming system
        //85 unit types, since 0x220-0x275
    TECH_TREE_AVAILABLE_UNIT_HEADER         = 0x26, // container for units availability for the player
    TECH_TREE_UNIT_TYPE_COUNT               = 0x27, // number of available unit types (4 bytes)
    TECH_TREE_UNIT_TYPE_NAMES               = 0x56, // names of available unit templates (512 bytes/building)
        //350 building types, since 0x34-0x192
    TECH_TREE_AVAILABLE_BUILDINGS_HEADER    = 0x4C, // container for building availability for the player
    TECH_TREE_BUILDING_TYPE_COUNT           = 0x4D, // number of available building types (4 bytes)
    TECH_TREE_BUILDING_TYPE_NAMES           = 0x4E, // names of available building templates (512 bytes/building)

    //resource system
    RESOURCE_HEADER                         = 0x2C, // resource header
    RESOURCE_COUNT                          = 0x2D, // number of resources (4 bytes)
    RESOURCE_ENTRIES                        = 0x2E, // resource data (8 bytes each)

    //variable data system
    VARIABLE_DATA_CHUNK                     = 0x08, // variable-size chunk with some conditional fields
};

struct ChunkTypeInfo {
    ChunkType type;
    const char* name;
};

extern const ChunkTypeInfo ChunkTypeTable[];
extern const std::unordered_map<ChunkType, std::string> ChunkTypeNames;

//get human-readable name for chunk type, i.e mapping to a string (prob a better way of doing this)
std::string getChunkTypeName(ChunkType type);
