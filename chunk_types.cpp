#include "chunk_types.h"

const ChunkTypeInfo ChunkTypeTable[] = {
    { ChunkType::SCENARIO_HEADER,                       "SCENARIO_HEADER_DATA" },
    { ChunkType::METADATA,                              "METADATA_DATA" },

    { ChunkType::MAP_DATA,                              "MAP_DATA" },
    { ChunkType::MAP_STRUCTURE,                         "MAP_STRUCTURE" },
    { ChunkType::TILE_PROPERTIES,                       "TILE_PROPERTIES" },
    { ChunkType::TERRAIN_LAYOUT_DATA,                   "TERRAIN_LAYOUT_DATA" },
    { ChunkType::TERRAIN_TYPE,                          "TERRAIN_TYPE" },
    { ChunkType::MAP_STRUCTURE_HEADER,                  "MAP_STRUCTURE_HEADER" },
    { ChunkType::TERRAIN_LAYOUT_HEADER,                 "TERRAIN_LAYOUT_HEADER" },
    { ChunkType::MAP_NAME_DATA,                         "MAP_NAME_DATA" },
    { ChunkType::MAP_OBJECTS,                           "MAP_OBJECTS" },
    { ChunkType::OBJECT_GROUPS,                         "OBJECT_GROUPS" },
    { ChunkType::MAP_LINKS,                             "MAP_LINKS" },
    { ChunkType::VISIBILITY_HEADER,                     "VISIBILITY_HEADER" },
    { ChunkType::VISIBILITY_DATA_COUNT,                 "VISIBILITY_DATA_COUNT" },
    { ChunkType::TILE_VISIBILITY,                       "TILE_VISIBILITY" },
    { ChunkType::MAP_METADATA_HEADER,                   "MAP_METADATA_HEADER" },
    { ChunkType::MAP_METADATA_DATA,                     "MAP_METADATA_DATA" },
    { ChunkType::EXTENDED_MAP_DATA,                     "EXTENDED_MAP_DATA" },

    { ChunkType::ADV_FEATURE_1,                         "ADV_FEATURE_1" },
    { ChunkType::FEATURE_COUNT,                         "FEATURE_COUNT" },
    { ChunkType::FEATURE_DATA,                          "FEATURE_DATA" },
    { ChunkType::ADV_FEATURE_4,                         "ADV_FEATURE_4" },
    { ChunkType::LOCATION_COUNT,                        "LOCATION_COUNT" },
    { ChunkType::LOCATION_DATA,                         "LOCATION_DATA" },
    { ChunkType::ADV_FEATURE_2,                         "ADV_FEATURE_2" },
    { ChunkType::SPATIAL_GROUP_COUNT,                   "SPATIAL_GROUP_COUNT" },
    { ChunkType::GROUP_COORDINATE_COUNT,                "GROUP_COORDINATE_COUNT" },
    { ChunkType::COORDINATE_DATA,                       "COORDINATE_DATA" },
    { ChunkType::ADV_FEATURE_3,                         "ADV_FEATURE_3" },
    { ChunkType::ENTITY_COUNT,                          "ENTITY_COUNT" },
    { ChunkType::ENTITY_DATA,                           "ENTITY_DATA" },

    { ChunkType::TRIGGER_DATA,                          "TRIGGER_DATA" },
    { ChunkType::TRIGGER_COUNT,                         "TRIGGER_COUNT" },
    { ChunkType::TRIGGER_ENTRIES,                       "TRIGGER_ENTRIES" },

    { ChunkType::MAP_RESOURCES_DATA,                    "MAP_RESOURCES_DATA" },
    { ChunkType::MAP_RESOURCES_COUNT,                   "MAP_RESOURCES_COUNT" },
    { ChunkType::MAP_RESOURCES_ENTRIES,                 "MAP_RESOURCES_ENTRIES" },

    { ChunkType::FORMATION_DATA,                        "FORMATION_DATA" },
    { ChunkType::FORMATION_HEADER,                      "FORMATION_HEADER" },
    { ChunkType::FORMATION_PROPERTIES,                  "FORMATION_PROPERTIES" },
    { ChunkType::BUILDING_GROUP_HEADER,                 "BUILDING_GROUP_HEADER" },
    { ChunkType::BUILDING_GROUP_PROPERTIES,             "BUILDING_GROUP_PROPERTIES" },

    { ChunkType::GARRISON_HEADER,                       "GARRISON_HEADER" },
    { ChunkType::GARRISON_COUNT,                        "GARRISON_COUNT" },
    { ChunkType::GARRISON_UNIT_NAMES,                   "GARRISON_UNIT_NAMES" },

    { ChunkType::EDITOR_DATA,                           "EDITOR_DATA" },
    { ChunkType::EDITOR_PLAYER_NATION_DATA,             "EDITOR_PLAYER_NATION_DATA" },
    { ChunkType::EDITOR_MAP_TERRAIN_DATA,               "EDITOR_MAP_TERRAIN_DATA" },
    { ChunkType::EDITOR_UNIT_PLACEMENT_STATE,           "EDITOR_UNIT_PLACEMENT_STATE" },
    { ChunkType::EDITOR_RESOURCE_DATA,                  "EDITOR_RESOURCE_DATA" },
    { ChunkType::EDITOR_RESEARCH_PROGRESS,              "EDITOR_RESEARCH_PROGRESS" },

    { ChunkType::CITY_BUILDING_CONTAINER,               "BUILDING_DATA" },
    { ChunkType::CITY_BUILDING_HEADER,                  "BUILDING_HEADER" },
    { ChunkType::CITY_BUILDING_PROPERTIES,              "BUILDING_PROPERTIES" },

    { ChunkType::DIPLOMACY_DATA,                        "DIPLOMACY_DATA" },
    { ChunkType::DIPLOMATIC_COUNT,                      "DIPLOMATIC_COUNT" },
    { ChunkType::DIPLOMATIC_ENTRIES,                    "DIPLOMATIC_ENTRIES" },

    { ChunkType::OBJECT_DATA,                           "OBJECT_DATA" },
    { ChunkType::OBJECT_HEADER,                         "OBJECT_HEADER" },
    { ChunkType::UNIT_OBJECT,                           "UNIT_OBJECT" },
    { ChunkType::STRUCTURE_OBJECT,                      "STRUCTURE_OBJECT" },
    { ChunkType::RESOURCE_OBJECT,                       "RESOURCE_OBJECT" },

    { ChunkType::PLAYER_DATA,                           "PLAYER_DATA" },
    { ChunkType::PLAYER_DATA_START,                     "PLAYER_DATA_START" },
    { ChunkType::PLAYER_STATUS_HEADER,                  "PLAYER_STATUS_HEADER" },
    { ChunkType::ACTIVE_PLAYER_COUNT,                   "ACTIVE_PLAYER_COUNT" },
    { ChunkType::PLAYER_DETAILS,                        "PLAYER_DETAILS" },
    { ChunkType::PLAYER_PROPERTIES,                     "PLAYER_PROPERTIES" },

    { ChunkType::TECH_TREE_AVAILABLE_UNIT_HEADER,       "TECH_TREE_AVAILABLE_UNIT_HEADER" },
    { ChunkType::TECH_TREE_UNIT_TYPE_COUNT,             "TECH_TREE_UNIT_TYPE_COUNT" },
    { ChunkType::TECH_TREE_UNIT_TYPE_NAMES,             "TECH_TREE_UNIT_TYPE_NAMES" },
    { ChunkType::TECH_TREE_AVAILABLE_BUILDINGS_HEADER,  "TECH_TREE_AVAILABLE_BUILDINGS_HEADER" },
    { ChunkType::TECH_TREE_BUILDING_TYPE_COUNT,         "TECH_TREE_BUILDING_TYPE_COUNT" },
    { ChunkType::TECH_TREE_BUILDING_TYPE_NAMES,         "TECH_TREE_BUILDING_TYPE_NAMES" },

    { ChunkType::RESOURCE_HEADER,                       "RESOURCE_HEADER" },
    { ChunkType::RESOURCE_COUNT,                        "RESOURCE_COUNT" },
    { ChunkType::RESOURCE_ENTRIES,                      "RESOURCE_ENTRIES" }
};

const std::unordered_map<ChunkType, std::string> ChunkTypeNames = [] {
    std::unordered_map<ChunkType, std::string> m;
    for (const auto& entry : ChunkTypeTable) {
        m.emplace(static_cast<ChunkType>(entry.type), entry.name);
    }
    return m;
}();

std::string getChunkTypeName(ChunkType type) {
    auto it = ChunkTypeNames.find(type);
    if (it != ChunkTypeNames.end()) {
        return it->second;
    }
    return "UNKNOWN"; //If we have missed a chunk id (lack of testing in scenario editor) it will print UNKNOWN
}
