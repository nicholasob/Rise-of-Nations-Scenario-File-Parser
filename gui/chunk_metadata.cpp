#include "chunk_metadata.h"
#include "../include/base/chunk_reflection.h"
#include "../include/chunks/player_chunks.h"
#include "../include/chunks/map_chunks.h"
#include "../include/chunks/unit_chunks.h"
#include <QColor>

ChunkMetadata& ChunkMetadata::instance()
{
    static ChunkMetadata inst;
    return inst;
}

ChunkMetadata::ChunkMetadata()
    : colorMap(QColor(173, 216, 230, 80))        // Light blue
    , colorPlayer(QColor(144, 238, 144, 80))     // Light green
    , colorUnits(QColor(255, 200, 124, 80))      // Light orange
    , colorAdvanced(QColor(221, 160, 221, 80))   // Plum
    , colorMetadata(QColor(211, 211, 211, 80))   // Light gray
    , colorEditor(QColor(255, 228, 181, 80))     // Moccasin
    , colorDiplomacy(QColor(255, 182, 193, 80))  // Light pink
    , colorResource(QColor(240, 230, 140, 80))   // Khaki
    , colorTrigger(QColor(176, 224, 230, 80))    // Powder blue
    , colorVisibility(QColor(216, 191, 216, 80)) // Thistle
    , colorContainer(QColor(245, 245, 245, 60))  // White smoke (lighter for containers)
{
    registerChunks();
}

void ChunkMetadata::registerChunk(const ChunkInfo& info)
{
    m_chunkRegistry[info.type] = info;
}

void ChunkMetadata::registerChunks()
{
    // Container chunks
    registerChunk(ChunkInfo(ChunkType::SCENARIO_HEADER, "SCENARIO_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::MAP_DATA, "MAP_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::TRIGGER_DATA, "TRIGGER_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::MAP_RESOURCES_DATA, "MAP_RESOURCES_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::FORMATION_DATA, "FORMATION_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::FORMATION_HEADER, "FORMATION_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::BUILDING_GROUP_HEADER, "BUILDING_GROUP_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::GARRISON_HEADER, "GARRISON_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::CITY_BUILDING_CONTAINER, "CITY_BUILDING_CONTAINER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::CITY_BUILDING_HEADER, "CITY_BUILDING_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::DIPLOMACY_DATA, "DIPLOMACY_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::OBJECT_DATA, "OBJECT_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::OBJECT_HEADER, "OBJECT_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::PLAYER_DATA, "PLAYER_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::PLAYER_DATA_START, "PLAYER_DATA_START", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::PLAYER_STATUS_HEADER, "PLAYER_STATUS_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::RESOURCE_HEADER, "RESOURCE_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::EDITOR_DATA, "EDITOR_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::EDITOR_PLAYER_NATION_DATA, "EDITOR_PLAYER_NATION_DATA", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::EDITOR_UNIT_PLACEMENT_STATE, "EDITOR_UNIT_PLACEMENT_STATE", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::MAP_STRUCTURE_HEADER, "MAP_STRUCTURE_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::TERRAIN_LAYOUT_HEADER, "TERRAIN_LAYOUT_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::ADV_FEATURE_1, "ADV_FEATURE_1", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::ADV_FEATURE_2, "ADV_FEATURE_2", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::ADV_FEATURE_3, "ADV_FEATURE_3", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::ADV_FEATURE_4, "ADV_FEATURE_4", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::TECH_TREE_AVAILABLE_UNIT_HEADER, "TECH_TREE_AVAILABLE_UNIT_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::TECH_TREE_AVAILABLE_BUILDINGS_HEADER, "TECH_TREE_AVAILABLE_BUILDINGS_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::VISIBILITY_HEADER, "VISIBILITY_HEADER", "Container", 0, colorContainer, true));
    registerChunk(ChunkInfo(ChunkType::MAP_METADATA_HEADER, "MAP_METADATA_HEADER", "Container", 0, colorContainer, true));

    // Map chunks
    registerReflectedChunk<MapStructureChunk0xf>(
        ChunkType::MAP_STRUCTURE,
        "MAP_STRUCTURE",
        "Map",
        sizeof(MapStructureChunk0xf),
        colorMap
    );

    ChunkInfo tileProps(ChunkType::TILE_PROPERTIES, "TILE_PROPERTIES", "Map", 0, colorMap);
    tileProps.fields.push_back(FieldInfo("properties", 0, 4, "uint32_t[]", "4 bytes per tile"));
    registerChunk(tileProps);

    registerReflectedChunk<TerrainLayoutChunk0x11>(
        ChunkType::TERRAIN_LAYOUT_DATA,
        "TERRAIN_LAYOUT_DATA",
        "Map",
        sizeof(TerrainLayoutChunk0x11),
        colorMap
    );

    ChunkInfo terrainType(ChunkType::TERRAIN_TYPE, "TERRAIN_TYPE", "Map", 0, colorMap);
    terrainType.fields.push_back(FieldInfo("terrain_type", 0, 2, "uint16_t[]", "2 bytes per position"));
    registerChunk(terrainType);

    registerReflectedChunk<MapNameChunk0x51>(
        ChunkType::MAP_NAME_DATA,
        "MAP_NAME_DATA",
        "Map",
        sizeof(MapNameChunk0x51),
        colorMap
    );

    registerChunk(ChunkInfo(ChunkType::MAP_OBJECTS, "MAP_OBJECTS", "Map", 0, colorMap));
    registerChunk(ChunkInfo(ChunkType::OBJECT_GROUPS, "OBJECT_GROUPS", "Map", 0, colorMap));
    registerChunk(ChunkInfo(ChunkType::MAP_LINKS, "MAP_LINKS", "Map", 0, colorMap));

    // Metadata chunk - ScenarioHeader0x6C (384 bytes)
    ChunkInfo metadata(ChunkType::METADATA, "METADATA", "Metadata", 384, colorMetadata);
    metadata.fields.push_back(FieldInfo("version", 0, 4, "uint32_t", "Version or chunk count"));
    metadata.fields.push_back(FieldInfo("magic", 4, 4, "uint32_t", "Magic number (0xf00dcafe)"));
    metadata.fields.push_back(FieldInfo("scenario_name", 8, 200, "char16_t[100]", "Scenario name (UTF-16)"));
    metadata.fields.push_back(FieldInfo("player_setting1", 208, 4, "uint32_t", "Player setting 1"));
    metadata.fields.push_back(FieldInfo("game_rules", 212, 4, "uint32_t", "Player setting 2"));
    metadata.fields.push_back(FieldInfo("player_setting3", 216, 4, "uint32_t", "Player setting 3"));
    metadata.fields.push_back(FieldInfo("player_setting4", 220, 4, "uint32_t", "Player setting 4"));
    metadata.fields.push_back(FieldInfo("global_setting", 224, 4, "uint32_t", "Global setting"));
    metadata.fields.push_back(FieldInfo("game_flags", 228, 4, "uint32_t", "Game mode flags"));
    metadata.fields.push_back(FieldInfo("ai_difficulty", 232, 4, "uint32_t", "AI difficulty"));
    metadata.fields.push_back(FieldInfo("start_age", 236, 4, "uint32_t", "Victory condition type"));
    metadata.fields.push_back(FieldInfo("end_age", 240, 4, "uint32_t", "Map type"));
    metadata.fields.push_back(FieldInfo("reveal_map", 244, 4, "uint32_t", "Game setting 1"));
    metadata.fields.push_back(FieldInfo("elimination", 248, 4, "uint32_t", "Game setting 2"));
    metadata.fields.push_back(FieldInfo("victory", 252, 4, "uint32_t", "Game setting 3"));
    metadata.fields.push_back(FieldInfo("score", 256, 4, "uint32_t", "Game setting 4"));
    metadata.fields.push_back(FieldInfo("time_limit", 260, 4, "uint32_t", "Game setting 5"));
    metadata.fields.push_back(FieldInfo("wonder_victory", 264, 4, "uint32_t", "Game setting 6"));
    metadata.fields.push_back(FieldInfo("territory_goal", 268, 4, "uint32_t", "Game setting 7"));
    metadata.fields.push_back(FieldInfo("income_goal", 272, 4, "uint32_t", "Game setting 8"));
    metadata.fields.push_back(FieldInfo("player_setting5", 276, 4, "uint32_t", "Player setting 5"));
    // Global blobs (280-376 = 96 bytes = 24 uint32_t values)
    for (int i = 0; i < 8; ++i) {
        metadata.fields.push_back(FieldInfo("camera_start_position_X_nation_" + std::to_string(i+1), 280 + i*4, 4, "uint32_t", "Global setting " + std::to_string(i+1)));
    }
    // Global blobs (280-376 = 96 bytes = 24 uint32_t values)
    for (int i = 0; i < 8; ++i) {
        metadata.fields.push_back(FieldInfo("camera_start_position_Y_nation_" + std::to_string(i+1), 280 + i*4, 4, "uint32_t", "Global setting " + std::to_string(i+1)));
    }
    // Global blobs (280-376 = 96 bytes = 24 uint32_t values)
    for (int i = 0; i < 8; ++i) {
        metadata.fields.push_back(FieldInfo("global_" + std::to_string(i+1), 280 + i*4, 4, "uint32_t", "Global setting " + std::to_string(i+1)));
    }
    metadata.fields.push_back(FieldInfo("some_setting", 376, 4, "uint32_t", "Additional setting"));
    metadata.fields.push_back(FieldInfo("Population", 380, 4, "uint32_t", "Extra flag"));
    registerChunk(metadata);

    // Player chunks
    registerReflectedChunk<PlayerCountChunk0x2a>(
        ChunkType::ACTIVE_PLAYER_COUNT,
        "ACTIVE_PLAYER_COUNT",
        "Player",
        sizeof(PlayerCountChunk0x2a),
        colorPlayer
    );

    // PlayerEntry (12 bytes each) - variable count
    registerReflectedChunk<PlayerEntry>(
        ChunkType::PLAYER_DETAILS,
        "PLAYER_DETAILS",
        "Player",
        sizeof(PlayerEntry),
        colorPlayer
    );

    // PlayerPropertiesChunk0x6B (236 bytes each)
    registerReflectedChunk<PlayerPropertiesChunk0x6B>(
        ChunkType::PLAYER_PROPERTIES,
        "PLAYER_PROPERTIES",
        "Player",
        sizeof(PlayerPropertiesChunk0x6B),
        colorPlayer
    );

    // Unit/Formation chunks - FormationChunk0x53 (580 bytes)
    registerReflectedChunk<FormationChunk0x53>(
        ChunkType::FORMATION_PROPERTIES,
        "FORMATION_PROPERTIES",
        "Units",
        sizeof(FormationChunk0x53),
        colorUnits
    );

    // BuildingPropertiesChunk0x54 (548 bytes)
    ChunkInfo buildingProps(ChunkType::BUILDING_GROUP_PROPERTIES, "BUILDING_GROUP_PROPERTIES", "Units", 548, colorUnits);
    buildingProps.fields.push_back(FieldInfo("player_index", 0, 4, "int32_t", "Player owning entity (0-7)"));
    buildingProps.fields.push_back(FieldInfo("entity_index", 4, 4, "int32_t", "Entity index"));
    buildingProps.fields.push_back(FieldInfo("additional_data", 8, 4, "uint32_t", "Additional entity data"));
    buildingProps.fields.push_back(FieldInfo("unknown_field_1", 12, 4, "uint32_t", "Always 0xFFFFFFFF"));
    buildingProps.fields.push_back(FieldInfo("unknown_field_2", 16, 4, "uint32_t", "Always 0xFFFFFFFF"));
    buildingProps.fields.push_back(FieldInfo("encrypted_param_1", 20, 4, "uint32_t", "Health/armor/production (XOR 0x63637)"));
    buildingProps.fields.push_back(FieldInfo("encrypted_param_2", 24, 4, "uint32_t", "Attack/range/capacity (XOR 0x63637)"));
    buildingProps.fields.push_back(FieldInfo("entity_name", 28, 512, "char16_t[256]", "Entity/building name (UTF-16)"));
    buildingProps.fields.push_back(FieldInfo("capability_type_flag", 540, 4, "uint32_t", "Entity capability/type flags"));
    buildingProps.fields.push_back(FieldInfo("additional_byte_flag", 544, 1, "uint8_t", "Additional entity-specific flag"));
    buildingProps.fields.push_back(FieldInfo("padding", 545, 3, "uint8_t[3]", "Alignment padding"));
    registerChunk(buildingProps);

    // GarrisonCountHeaderChunk0x21 (8 bytes)
    ChunkInfo garrisonCount(ChunkType::GARRISON_COUNT, "GARRISON_COUNT", "Units", 8, colorUnits);
    garrisonCount.fields.push_back(FieldInfo("unit_count", 0, 4, "int32_t", "Number of units stationed"));
    garrisonCount.fields.push_back(FieldInfo("garrison_type_flag", 4, 4, "uint32_t", "Garrison capabilities flag"));
    registerChunk(garrisonCount);

    // GarrisonUnitNamesDataType (512 bytes each)
    ChunkInfo garrisonNames(ChunkType::GARRISON_UNIT_NAMES, "GARRISON_UNIT_NAMES", "Units", 512, colorUnits);
    garrisonNames.fields.push_back(FieldInfo("unit_name", 0, 512, "char16_t[256]", "Unit name (UTF-16)"));
    registerChunk(garrisonNames);

    // Object chunks - UnitsObjectDataChunk0x3A (804 bytes)
    ChunkInfo unitObject(ChunkType::UNIT_OBJECT, "UNIT_OBJECT", "Objects", 804, colorUnits);
    unitObject.fields.push_back(FieldInfo("job_time", 0, 4, "uint32_t", "From offset 0x08"));
    unitObject.fields.push_back(FieldInfo("property_0x04", 4, 4, "uint32_t", "Property from offset 0x04"));
    unitObject.fields.push_back(FieldInfo("property_0x10", 8, 4, "uint32_t", "Property from offset 0x10"));
    unitObject.fields.push_back(FieldInfo("unit_name", 12, 512, "char16_t[256]", "Unit name/description (UTF-16)"));
    unitObject.fields.push_back(FieldInfo("string_data_2", 524, 200, "char16_t[100]", "Secondary string data (UTF-16)"));
    unitObject.fields.push_back(FieldInfo("required_1", 724, 4, "uint32_t", "Required field 1"));
    unitObject.fields.push_back(FieldInfo("required_2", 728, 4, "uint32_t", "Required field 2"));
    unitObject.fields.push_back(FieldInfo("required_3", 732, 4, "uint32_t", "Required field 3"));
    unitObject.fields.push_back(FieldInfo("food_cost", 736, 4, "uint32_t", "Food cost"));
    unitObject.fields.push_back(FieldInfo("timber_cost", 740, 4, "uint32_t", "Timber cost"));
    unitObject.fields.push_back(FieldInfo("wealth_cost", 744, 4, "uint32_t", "Wealth cost"));
    unitObject.fields.push_back(FieldInfo("knowledge_cost", 748, 4, "uint32_t", "Knowledge cost"));
    unitObject.fields.push_back(FieldInfo("metal_cost", 752, 4, "uint32_t", "Metal cost"));
    unitObject.fields.push_back(FieldInfo("oil_cost", 756, 4, "uint32_t", "Oil cost"));
    unitObject.fields.push_back(FieldInfo("attack", 760, 4, "uint32_t", "Attack value"));
    unitObject.fields.push_back(FieldInfo("min_range", 764, 4, "uint32_t", "Minimum range"));
    unitObject.fields.push_back(FieldInfo("max_range", 768, 4, "uint32_t", "Maximum range"));
    unitObject.fields.push_back(FieldInfo("hits", 772, 4, "uint32_t", "Hit points"));
    unitObject.fields.push_back(FieldInfo("armor", 776, 4, "uint32_t", "Armor value"));
    unitObject.fields.push_back(FieldInfo("line_of_sight", 780, 4, "uint32_t", "Line of sight"));
    unitObject.fields.push_back(FieldInfo("science_LoS", 784, 4, "uint32_t", "Science line of sight"));
    unitObject.fields.push_back(FieldInfo("moves", 788, 4, "uint32_t", "Movement points"));
    unitObject.fields.push_back(FieldInfo("turn_speed", 792, 4, "uint32_t", "Turn speed"));
    unitObject.fields.push_back(FieldInfo("craft", 796, 4, "uint32_t", "Craft value"));
    unitObject.fields.push_back(FieldInfo("control_cost", 800, 4, "uint32_t", "Control cost"));
    registerChunk(unitObject);

    // BuildingObjectDataChunk0x3B (816 bytes)
    ChunkInfo structureObject(ChunkType::STRUCTURE_OBJECT, "STRUCTURE_OBJECT", "Objects", 816, colorUnits);
    structureObject.fields.push_back(FieldInfo("job_time", 0, 4, "uint32_t", "From offset 0x08"));
    structureObject.fields.push_back(FieldInfo("property_0x04", 4, 4, "uint32_t", "Property from offset 0x04"));
    structureObject.fields.push_back(FieldInfo("property_0x10", 8, 4, "uint32_t", "Property from offset 0x10"));
    structureObject.fields.push_back(FieldInfo("building_name", 12, 512, "char16_t[256]", "Building name/description (UTF-16)"));
    structureObject.fields.push_back(FieldInfo("string_data_2", 524, 200, "char16_t[100]", "Secondary string data (UTF-16)"));
    structureObject.fields.push_back(FieldInfo("required_1", 724, 4, "uint32_t", "Required field 1"));
    structureObject.fields.push_back(FieldInfo("required_2", 728, 4, "uint32_t", "Required field 2"));
    structureObject.fields.push_back(FieldInfo("required_3", 732, 4, "uint32_t", "Required field 3"));
    structureObject.fields.push_back(FieldInfo("food_cost", 736, 4, "uint32_t", "Food cost"));
    structureObject.fields.push_back(FieldInfo("timber_cost", 740, 4, "uint32_t", "Timber cost"));
    structureObject.fields.push_back(FieldInfo("wealth_cost", 744, 4, "uint32_t", "Wealth cost"));
    structureObject.fields.push_back(FieldInfo("knowledge_cost", 748, 4, "uint32_t", "Knowledge cost"));
    structureObject.fields.push_back(FieldInfo("metal_cost", 752, 4, "uint32_t", "Metal cost"));
    structureObject.fields.push_back(FieldInfo("oil_cost", 756, 4, "uint32_t", "Oil cost"));
    structureObject.fields.push_back(FieldInfo("attack", 760, 4, "uint32_t", "Attack value"));
    structureObject.fields.push_back(FieldInfo("min_range", 764, 4, "uint32_t", "Minimum range"));
    structureObject.fields.push_back(FieldInfo("max_range", 768, 4, "uint32_t", "Maximum range"));
    structureObject.fields.push_back(FieldInfo("hits", 772, 4, "uint32_t", "Hit points"));
    structureObject.fields.push_back(FieldInfo("armor", 776, 4, "uint32_t", "Armor value"));
    structureObject.fields.push_back(FieldInfo("line_of_sight", 780, 4, "uint32_t", "Line of sight"));
    structureObject.fields.push_back(FieldInfo("science_LoS", 784, 4, "uint32_t", "Science line of sight"));
    structureObject.fields.push_back(FieldInfo("town_hits", 788, 4, "uint32_t", "Town hit points"));
    structureObject.fields.push_back(FieldInfo("plunder_amount", 792, 4, "uint32_t", "Plunder amount"));
    structureObject.fields.push_back(FieldInfo("plunder_good", 796, 4, "uint32_t", "Plunder good type"));
    structureObject.fields.push_back(FieldInfo("garrison_max", 800, 4, "uint32_t", "Max garrison size"));
    structureObject.fields.push_back(FieldInfo("base_arrows", 804, 4, "uint32_t", "Base arrow count"));
    structureObject.fields.push_back(FieldInfo("most_shots", 808, 4, "uint32_t", "Max shots"));
    structureObject.fields.push_back(FieldInfo("wonder_points", 812, 4, "uint32_t", "Wonder points"));
    registerChunk(structureObject);

    // ResourceObjectDataChunk0x3C (760 bytes)
    ChunkInfo resourceObject(ChunkType::RESOURCE_OBJECT, "RESOURCE_OBJECT", "Objects", 760, colorResource);
    resourceObject.fields.push_back(FieldInfo("property_0x08", 0, 4, "uint32_t", "Property from offset 0x08"));
    resourceObject.fields.push_back(FieldInfo("property_0x04", 4, 4, "uint32_t", "Property from offset 0x04"));
    resourceObject.fields.push_back(FieldInfo("property_0x10", 8, 4, "uint32_t", "Property from offset 0x10"));
    resourceObject.fields.push_back(FieldInfo("resource_name", 12, 512, "char16_t[256]", "Resource name/description (UTF-16)"));
    resourceObject.fields.push_back(FieldInfo("string_data_2", 524, 200, "char16_t[100]", "Secondary string data (UTF-16)"));
    resourceObject.fields.push_back(FieldInfo("resource_pos_x", 724, 4, "uint32_t", "Resource X position"));
    resourceObject.fields.push_back(FieldInfo("resource_pos_y", 728, 4, "uint32_t", "Resource Y position"));
    resourceObject.fields.push_back(FieldInfo("resource_pos_z", 732, 4, "uint32_t", "Resource Z position"));
    resourceObject.fields.push_back(FieldInfo("resource_prop_18", 736, 4, "uint32_t", "Resource property 0x18"));
    resourceObject.fields.push_back(FieldInfo("resource_prop_1c", 740, 4, "uint32_t", "Resource property 0x1c"));
    resourceObject.fields.push_back(FieldInfo("resource_prop_20", 744, 4, "uint32_t", "Resource property 0x20"));
    resourceObject.fields.push_back(FieldInfo("resource_prop_24", 748, 4, "uint32_t", "Resource property 0x24"));
    resourceObject.fields.push_back(FieldInfo("resource_prop_28", 752, 4, "uint32_t", "Resource property 0x28"));
    resourceObject.fields.push_back(FieldInfo("resource_prop_2c", 756, 4, "uint32_t", "Resource property 0x2c"));
    registerChunk(resourceObject);

    // Building chunks - CityBuildingPropertiesChunk0x1e (752 bytes)
    ChunkInfo cityBuildingProps(ChunkType::CITY_BUILDING_PROPERTIES, "CITY_BUILDING_PROPERTIES", "Buildings", 752, colorUnits);
    cityBuildingProps.fields.push_back(FieldInfo("player_index", 0, 4, "int32_t", "Player who owns building (0-7)"));
    cityBuildingProps.fields.push_back(FieldInfo("building_index", 4, 4, "int32_t", "Building index (starts at 2000)"));
    cityBuildingProps.fields.push_back(FieldInfo("additional_data", 8, 4, "uint32_t", "Additional building data"));
    cityBuildingProps.fields.push_back(FieldInfo("encrypted_param_1", 12, 4, "uint32_t", "Health/armor/production (XOR 0x63637)"));
    cityBuildingProps.fields.push_back(FieldInfo("encrypted_param_2", 16, 4, "uint32_t", "Attack/range/capacity (XOR 0x63637)"));
    cityBuildingProps.fields.push_back(FieldInfo("city_flags", 20, 4, "uint32_t", "City status flags"));
    cityBuildingProps.fields.push_back(FieldInfo("city_id", 24, 4, "int32_t", "City ID (-1 if orphaned)"));
    cityBuildingProps.fields.push_back(FieldInfo("padding_or_unknown", 28, 8, "uint32_t[2]", "8 bytes padding"));
    cityBuildingProps.fields.push_back(FieldInfo("city_name", 36, 200, "char16_t[100]", "Associated city name (UTF-16)"));
    cityBuildingProps.fields.push_back(FieldInfo("building_name", 236, 512, "char16_t[256]", "Building name (UTF-16)"));
    cityBuildingProps.fields.push_back(FieldInfo("city_capability_flag", 748, 4, "int32_t", "Whether building can be part of city"));
    registerChunk(cityBuildingProps);

    // Resource chunks
    ChunkInfo resourceCount(ChunkType::RESOURCE_COUNT, "RESOURCE_COUNT", "Resources", 4, colorResource);
    resourceCount.fields.push_back(FieldInfo("resource_type_count", 0, 4, "uint32_t", "Number of resource types"));
    registerChunk(resourceCount);

    // ResourceEntry (8 bytes each)
    ChunkInfo resourceEntries(ChunkType::RESOURCE_ENTRIES, "RESOURCE_ENTRIES", "Resources", 8, colorResource);
    resourceEntries.fields.push_back(FieldInfo("resource_index", 0, 4, "uint32_t", "Resource type index (0-5)"));
    resourceEntries.fields.push_back(FieldInfo("value", 4, 4, "uint32_t", "Resource value"));
    registerChunk(resourceEntries);

    // Trigger chunks
    ChunkInfo triggerCount(ChunkType::TRIGGER_COUNT, "TRIGGER_COUNT", "Triggers", 4, colorTrigger);
    triggerCount.fields.push_back(FieldInfo("active_count", 0, 4, "uint32_t", "Number of active triggers"));
    registerChunk(triggerCount);

    // TriggerDataEntry0x3 (520 bytes each)
    ChunkInfo triggerEntry(ChunkType::TRIGGER_ENTRIES, "TRIGGER_ENTRIES", "Triggers", 520, colorTrigger);
    triggerEntry.fields.push_back(FieldInfo("trigger_name", 0, 512, "char16_t[256]", "Trigger name/description (UTF-16)"));
    triggerEntry.fields.push_back(FieldInfo("encrypted_param1", 512, 4, "uint32_t", "Conditions/timing (XOR encrypted)"));
    triggerEntry.fields.push_back(FieldInfo("encrypted_param2", 516, 4, "uint32_t", "Actions/targets (XOR encrypted)"));
    registerChunk(triggerEntry);

    // Map resources chunks
    ChunkInfo resourcesCount(ChunkType::MAP_RESOURCES_COUNT, "MAP_RESOURCES_COUNT", "Map Resources", 4, colorResource);
    resourcesCount.fields.push_back(FieldInfo("map_resource_count", 0, 4, "uint32_t", "Number of active map resources"));
    registerChunk(resourcesCount);

    // MapResourceData0x4 (520 bytes each)
    ChunkInfo resourcesEntry(ChunkType::MAP_RESOURCES_ENTRIES, "MAP_RESOURCES_ENTRIES", "Map Resources", 520, colorResource);
    resourcesEntry.fields.push_back(FieldInfo("name", 0, 512, "char16_t[256]", "Resource name (UTF-16)"));
    resourcesEntry.fields.push_back(FieldInfo("position_x", 512, 4, "uint32_t", "Economic parameter 1 (XOR encrypted)"));
    resourcesEntry.fields.push_back(FieldInfo("position_y", 516, 4, "uint32_t", "Economic parameter 2 (XOR encrypted)"));
    registerChunk(resourcesEntry);

    // Advanced feature chunks
    // AdvancedFeature1 - Mountain features
    ChunkInfo featureCount(ChunkType::FEATURE_COUNT, "FEATURE_COUNT", "Advanced", 4, colorAdvanced);
    featureCount.fields.push_back(FieldInfo("feature_count", 0, 4, "uint32_t", "Number of mountain features"));
    registerChunk(featureCount);

    // AdvancedFeature1MountainFeatureEntry (12 bytes each)
    ChunkInfo featureData(ChunkType::FEATURE_DATA, "FEATURE_DATA", "Advanced", 12, colorAdvanced);
    featureData.fields.push_back(FieldInfo("feature_id", 0, 4, "uint32_t", "Mountain/terrain feature ID"));
    featureData.fields.push_back(FieldInfo("mountain_flag1", 4, 1, "uint8_t", "Mountain-related flag 1"));
    featureData.fields.push_back(FieldInfo("mountain_flag2", 5, 1, "uint8_t", "Mountain-related flag 2"));
    featureData.fields.push_back(FieldInfo("padding", 6, 2, "uint16_t", "Alignment padding"));
    featureData.fields.push_back(FieldInfo("feature_properties", 8, 4, "uint32_t", "Feature properties"));
    registerChunk(featureData);

    // AdvancedFeature4 - Waypoints
    ChunkInfo locationCount(ChunkType::LOCATION_COUNT, "LOCATION_COUNT", "Advanced", 4, colorAdvanced);
    locationCount.fields.push_back(FieldInfo("waypoint_count", 0, 4, "int32_t", "Number of waypoints"));
    registerChunk(locationCount);

    // WaypointEntryChunk0x4b (12 bytes each)
    ChunkInfo locationData(ChunkType::LOCATION_DATA, "LOCATION_DATA", "Advanced", 12, colorAdvanced);
    locationData.fields.push_back(FieldInfo("x_coordinate", 0, 4, "int32_t", "X position"));
    locationData.fields.push_back(FieldInfo("y_coordinate", 4, 4, "int32_t", "Y position"));
    locationData.fields.push_back(FieldInfo("flags_and_id", 8, 4, "uint32_t", "Waypoint ID and status flags"));
    registerChunk(locationData);

    // AdvancedFeature2 - Spatial groups
    ChunkInfo spatialGroupCount(ChunkType::SPATIAL_GROUP_COUNT, "SPATIAL_GROUP_COUNT", "Advanced", 4, colorAdvanced);
    spatialGroupCount.fields.push_back(FieldInfo("group_count", 0, 4, "uint32_t", "Number of spatial groups"));
    registerChunk(spatialGroupCount);

    ChunkInfo groupCoordCount(ChunkType::GROUP_COORDINATE_COUNT, "GROUP_COORDINATE_COUNT", "Advanced", 4, colorAdvanced);
    groupCoordCount.fields.push_back(FieldInfo("coordinate_count", 0, 4, "uint32_t", "Number of coordinate entries"));
    registerChunk(groupCoordCount);

    // AdvancedFeature2CoordinateArrayChunk0x5F (8 bytes each)
    ChunkInfo coordinateData(ChunkType::COORDINATE_DATA, "COORDINATE_DATA", "Advanced", 8, colorAdvanced);
    coordinateData.fields.push_back(FieldInfo("x_processed", 0, 4, "uint32_t", "X coordinate after lookup table processing"));
    coordinateData.fields.push_back(FieldInfo("y_processed", 4, 4, "uint32_t", "Y coordinate after lookup table processing"));
    registerChunk(coordinateData);

    // AdvancedFeature3
    ChunkInfo entityCount(ChunkType::ENTITY_COUNT, "ENTITY_COUNT", "Advanced", 4, colorAdvanced);
    entityCount.fields.push_back(FieldInfo("entry_count", 0, 4, "int32_t", "Number of coordinate entries"));
    registerChunk(entityCount);

    // AdvancedFeature3Chunk62Data (16 bytes each)
    ChunkInfo entityData(ChunkType::ENTITY_DATA, "ENTITY_DATA", "Advanced", 16, colorAdvanced);
    entityData.fields.push_back(FieldInfo("field_0x40", 0, 4, "uint32_t", "Field 0x40 (float or int)"));
    entityData.fields.push_back(FieldInfo("field_0x44", 4, 4, "uint32_t", "Field 0x44 (float or int)"));
    entityData.fields.push_back(FieldInfo("field_0x38", 8, 4, "uint32_t", "Field 0x38 (float or int)"));
    entityData.fields.push_back(FieldInfo("field_0x3c", 12, 4, "uint32_t", "Field 0x3c (float or int)"));
    registerChunk(entityData);

    // Visibility chunks
    ChunkInfo visibilityCount(ChunkType::VISIBILITY_DATA_COUNT, "VISIBILITY_DATA_COUNT", "Visibility", 4, colorVisibility);
    visibilityCount.fields.push_back(FieldInfo("count", 0, 4, "uint32_t", "Visibility data count"));
    registerChunk(visibilityCount);

    registerChunk(ChunkInfo(ChunkType::TILE_VISIBILITY, "TILE_VISIBILITY", "Visibility", 0, colorVisibility));

    // Map metadata chunks
    ChunkInfo mapMetadataCount(ChunkType::MAP_METADATA_DATA, "MAP_METADATA_DATA", "Map", 4, colorMap);
    mapMetadataCount.fields.push_back(FieldInfo("count", 0, 4, "uint32_t", "Metadata count"));
    registerChunk(mapMetadataCount);

    registerChunk(ChunkInfo(ChunkType::EXTENDED_MAP_DATA, "EXTENDED_MAP_DATA", "Map", 0, colorMap));

    // Diplomacy chunks
    ChunkInfo diplomaticCount(ChunkType::DIPLOMATIC_COUNT, "DIPLOMATIC_COUNT", "Diplomacy", 4, colorDiplomacy);
    diplomaticCount.fields.push_back(FieldInfo("diplomatic_count", 0, 4, "uint32_t", "Number of diplomatic relations"));
    registerChunk(diplomaticCount);

    // DiplomaticEntryData (520 bytes each)
    ChunkInfo diplomaticEntry(ChunkType::DIPLOMATIC_ENTRIES, "DIPLOMATIC_ENTRIES", "Diplomacy", 520, colorDiplomacy);
    diplomaticEntry.fields.push_back(FieldInfo("agreement_name", 0, 512, "char16_t[256]", "Agreement name/description (UTF-16)"));
    diplomaticEntry.fields.push_back(FieldInfo("diplomatic_terms", 512, 4, "uint32_t", "Treaty duration, trade values, military support"));
    diplomaticEntry.fields.push_back(FieldInfo("diplomatic_status", 516, 4, "uint32_t", "Alliance level, agreement state, relationship"));
    registerChunk(diplomaticEntry);

    // Editor chunks - EditorOrderGroupChunk0x30 (520 bytes)
    ChunkInfo editorMapTerrain(ChunkType::EDITOR_MAP_TERRAIN_DATA, "EDITOR_MAP_TERRAIN_DATA", "Editor", 520, colorEditor);
    editorMapTerrain.fields.push_back(FieldInfo("group_config", 0, 4, "uint32_t", "Group specific settings"));
    editorMapTerrain.fields.push_back(FieldInfo("units_in_group", 4, 4, "uint32_t", "Number of units in group (0-128)"));
    editorMapTerrain.fields.push_back(FieldInfo("unit_ids", 8, 512, "uint32_t[128]", "Unit ID array (512 bytes)"));
    registerChunk(editorMapTerrain);

    ChunkInfo editorResourceData(ChunkType::EDITOR_RESOURCE_DATA, "EDITOR_RESOURCE_DATA", "Editor", 4, colorEditor);
    editorResourceData.fields.push_back(FieldInfo("element_count", 0, 4, "uint32_t", "Count of editor elements"));
    registerChunk(editorResourceData);

    // EditorElementDataStructure0x33 (16 bytes each)
    ChunkInfo editorResearch(ChunkType::EDITOR_RESEARCH_PROGRESS, "EDITOR_RESEARCH_PROGRESS", "Editor", 16, colorEditor);
    editorResearch.fields.push_back(FieldInfo("element_config", 0, 4, "int32_t", "Element configuration/flags"));
    editorResearch.fields.push_back(FieldInfo("element_type", 4, 4, "int32_t", "Element type/properties"));
    editorResearch.fields.push_back(FieldInfo("x_coordinate", 8, 4, "uint32_t", "Element X coordinate"));
    editorResearch.fields.push_back(FieldInfo("y_coordinate", 12, 4, "uint32_t", "Element Y coordinate"));
    registerChunk(editorResearch);

    // Tech tree chunks
    registerReflectedChunk<TechTreeUnitTypeCountChunk0x27>(
        ChunkType::TECH_TREE_UNIT_TYPE_COUNT,
        "TECH_TREE_UNIT_TYPE_COUNT",
        "Tech Tree",
        sizeof(TechTreeUnitTypeCountChunk0x27),
        colorMetadata
    );

    registerReflectedChunk<TechTreeTypeName>(
        ChunkType::TECH_TREE_UNIT_TYPE_NAMES,
        "TECH_TREE_UNIT_TYPE_NAMES",
        "Tech Tree",
        sizeof(TechTreeTypeName),
        colorMetadata
    );

    ChunkInfo techTreeBuildingCount(ChunkType::TECH_TREE_BUILDING_TYPE_COUNT, "TECH_TREE_BUILDING_TYPE_COUNT", "Tech Tree", 4, colorMetadata);
    techTreeBuildingCount.fields.push_back(FieldInfo("count", 0, 4, "uint32_t", "Number of building types"));
    registerChunk(techTreeBuildingCount);

    registerReflectedChunk<TechTreeTypeName>(
        ChunkType::TECH_TREE_BUILDING_TYPE_NAMES,
        "TECH_TREE_BUILDING_TYPE_NAMES",
        "Tech Tree",
        sizeof(TechTreeTypeName),
        colorMetadata
    );

    // Special chunk
    registerChunk(ChunkInfo(ChunkType::VARIABLE_DATA_CHUNK, "VARIABLE_DATA_CHUNK", "Special", 0, colorMetadata));
}

const ChunkInfo* ChunkMetadata::getChunkInfo(ChunkType type) const
{
    auto it = m_chunkRegistry.find(type);
    if (it != m_chunkRegistry.end()) {
        return &it->second;
    }
    return nullptr;
}

const FieldInfo* ChunkMetadata::getFieldAtOffset(ChunkType type, size_t offset) const
{
    const ChunkInfo* info = getChunkInfo(type);
    if (!info) {
        return nullptr;
    }

    for (const auto& field : info->fields) {
        if (offset >= field.offset && offset < field.offset + field.size) {
            return &field;
        }
    }

    return nullptr;
}

QColor ChunkMetadata::getColorForChunk(ChunkType type) const
{
    const ChunkInfo* info = getChunkInfo(type);
    if (info) {
        return info->color;
    }
    return QColor(255, 255, 255, 40); // Default light color
}

std::string ChunkMetadata::getCategory(ChunkType type) const
{
    const ChunkInfo* info = getChunkInfo(type);
    if (info) {
        return info->category;
    }
    return "Unknown";
}

bool ChunkMetadata::isRegistered(ChunkType type) const
{
    return m_chunkRegistry.find(type) != m_chunkRegistry.end();
}

std::vector<FieldInfo> ChunkMetadata::convertDescriptors(const std::vector<FieldDescriptor>& descriptors)
{
    std::vector<FieldInfo> fields;
    fields.reserve(descriptors.size());
    for (const auto& desc : descriptors) {
        fields.emplace_back(desc.name, desc.offset, desc.size, desc.type, desc.description);
    }
    return fields;
}

template<typename T>
void ChunkMetadata::registerReflectedChunk(ChunkType type, const std::string& name,
                                           const std::string& category, size_t size, QColor color)
{
    static_assert(has_reflection_v<T>, "Type must have reflection metadata (use BEGIN_FIELD_DESCRIPTORS macro)");

    ChunkInfo info(type, name, category, size, color);
    info.fields = convertDescriptors(T::getFieldDescriptors());
    registerChunk(info);
}
