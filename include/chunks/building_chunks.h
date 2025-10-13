#pragma once
#include "../base/byte_convertible.h"
#include "../common/common_types.h"
#include "../common/encryption.h"
#include <cstdint>

#pragma pack(push, 1)
struct TechTreeBuildingTypeCountChunk0x4d : public ByteConvertible<TechTreeBuildingTypeCountChunk0x4d> {
    uint32_t building_type_count; //number of active tech tree building types
};
#pragma pack(pop)
static_assert(sizeof(TechTreeBuildingTypeCountChunk0x4d) == 0x4, "TechTreeBuildingTypeCountChunk0x4d must be exactly 4 bytes");

#pragma pack(push, 1)
struct TechTreeBuildingTypeNameChunk0x4e : public VariableLengthArrayChunk<TechTreeBuildingTypeNameChunk0x4e, TechTreeTypeName> {
    std::vector<TechTreeTypeName> unit_names;

    TechTreeBuildingTypeNameChunk0x4e() = default;

    TechTreeBuildingTypeNameChunk0x4e(const TechTreeBuildingTypeCountChunk0x4d& header) {
        unit_names.resize(header.building_type_count);
    }

    TechTreeBuildingTypeNameChunk0x4e(const size_t& unit_count) {
        unit_names.resize(unit_count);
    }

    std::vector<TechTreeTypeName>& get_container() override {
        return unit_names;
    }
    
    const std::vector<TechTreeTypeName>& get_container() const override {
        return unit_names;
    }
};
#pragma pack(pop)

//building properties data (752 bytes = 0x2f0)
#pragma pack(push,1)
struct CityBuildingPropertiesChunk0x1e : public ByteConvertible<CityBuildingPropertiesChunk0x1e> {
    int32_t  player_index;              // +0x00: player who owns this building (0-7)
    int32_t  building_index;            // +0x04: building index (starts at 2000)  
    uint32_t additional_data;           // +0x08: Additional building data from offset +0x24
    uint32_t encrypted_param_1;         // +0x0C: Health/armor/production (XOR 0x63637)
    uint32_t encrypted_param_2;         // +0x10: Attack/range/capacity (XOR 0x63637)
    uint32_t city_flags;                // +0x14: City status flags (bit 1, bit 2)  
    int32_t  city_id;                   // +0x18: City ID (-1 if orphaned building)
    char16_t city_name[100];            // +0x1C: associated city name (200 bytes)
    char16_t  building_name[256];        // +0xE8: Building name (512 bytes, wide char)
    int32_t  city_capability_flag;      // +0x2E8: Whether building can be part of city (local_5c)
    uint32_t padding_or_unknown[2];     // +0x2EC: 8 bytes to reach exactly 752 bytes total
    //Total struct size: 0x2F0 bytes (752 bytes)
};
#pragma pack(pop)
static_assert(sizeof(CityBuildingPropertiesChunk0x1e) == 0x2F0, "CityBuildingPropertiesChunk0x1e header size mismatch");

//garrison header (8 bytes)
#pragma pack(push,1)
struct GarrisonCountHeaderChunk0x21 : public ByteConvertible<GarrisonCountHeaderChunk0x21> {
    int32_t  unit_count;                // number of units stationed in building
    uint32_t garrison_type_flag;        // garrison capabilities flag
};
#pragma pack(pop)
static_assert(sizeof(GarrisonCountHeaderChunk0x21) == 0x8, "GarrisonCountHeaderChunk0x21 header size mismatch");

//Individual garrison unit name entry (512 bytes each)
struct GarrisonUnitNamesDataType {
    char16_t unit_name[256];             //unit name (512 bytes, wide char)
};

#pragma pack(push,1)
struct GarrisonUnitNames0x23 : public VariableLengthArrayChunk<GarrisonUnitNames0x23, GarrisonUnitNamesDataType> {
    std::vector<GarrisonUnitNamesDataType> garrison_names;

    GarrisonUnitNames0x23() = default;

    GarrisonUnitNames0x23(const GarrisonCountHeaderChunk0x21& header) {
        garrison_names.resize(header.unit_count);
    }

    GarrisonUnitNames0x23(const size_t& trigger_count) {
        garrison_names.resize(trigger_count);
    }

    std::vector<GarrisonUnitNamesDataType>& get_container() override {
        return garrison_names;
    }
    
    const std::vector<GarrisonUnitNamesDataType>& get_container() const override {
        return garrison_names;
    }
};
#pragma pack(pop)

//Formation/Special Entity data (548 bytes = 0x224)
// this structure represents entities in two contexts:
//      Phase 2: inactive buildings (bit 0x20 CLEAR) with garrison data
//      Phase 3: special formations/entities (index 3000+)
#pragma pack(push,1)
struct BuildingPropertiesChunk0x54 : public ByteConvertible<BuildingPropertiesChunk0x54> {
    int32_t  player_index;              // +0x00: player who owns this entity (0-7)
    int32_t  entity_index;              // +0x04: entity index (building or special formation)
    uint32_t additional_data;           // +0x08: Additional entity data from offset +0x24
    uint32_t unknown_field_1;           // +0x0C: Always 0xFFFFFFFF (for special entities)
    uint32_t unknown_field_2;           // +0x10: Always 0xFFFFFFFF (for special entities)  
    uint32_t encrypted_param_1;         // +0x14: Health/armor/production (XOR 0x63637)
    uint32_t encrypted_param_2;         // +0x18: Attack/range/capacity (XOR 0x63637)
    char16_t  entity_name[256];          // +0x1C: Entity/building name (512 bytes, wide char)
    uint32_t capability_type_flag;      // +0x21C: Entity capability/type flags
    uint8_t  additional_byte_flag;      // +0x220: Additional entity-specific flag  
    uint8_t  padding[3];                // +0x221: Alignment padding to reach 548 bytes
    // Total: 0x224 bytes (548 bytes)
};
#pragma pack(pop)
static_assert(sizeof(BuildingPropertiesChunk0x54) == 0x224, "BuildingPropertiesChunk0x54 header size mismatch");

//building data payload (816 bytes exactly - 0x330)
#pragma pack(push, 1)
struct BuildingObjectDataChunk0x3B : public ByteConvertible<BuildingObjectDataChunk0x3B> {
    //basic building properties (written by FUN_00667ee0 - same as units)
    uint32_t job_time;     // +0x00: From source offset 0x08
    uint32_t property_0x04;     // +0x04: From source offset 0x04  
    uint32_t property_0x10;     // +0x08: From source offset 0x10
    
    //string area 1 (variable length, max 512 bytes)
    char16_t string_data_1[256]; // +0x0C: Building name/description (max 255 chars + null)
    
    //string area 2 (variable length, max 200 bytes)  
    char16_t string_data_2[100]; // +0x20C: Secondary string data (max 99 chars + null)
    
    // fixed position building properties (written by FUN_00667ee0) based on scenario editor
    uint32_t required_1;    // +0x2D4: param_2[0xb5] = source[0x30]
    uint32_t required_2;    // +0x2D8: param_2[0xb6] = source[0x34] 
    uint32_t required_3;    // +0x2DC: param_2[0xb7] = source[0x38]
    uint32_t food_cost;  // +0x2E0: param_2[0xb8] = source[0x18]
    uint32_t timber_cost;  // +0x2E4: param_2[0xb9] = source[0x1c]
    uint32_t wealth_cost;  // +0x2E8: param_2[0xba] = source[0x20]
    uint32_t knowledge_cost;  // +0x2EC: param_2[0xbb] = source[0x24]
    uint32_t metal_cost;  // +0x2F0: param_2[0xbc] = source[0x28]
    uint32_t oil_cost;  // +0x2F4: param_2[0xbd] = source[0x2c]
    
    // additional building properties (written by vtable method)
    uint32_t attack;  // +0x2F8: From source piVar1[0x7a]
    uint32_t min_range;  // +0x2FC: From source piVar1[0x7e]
    uint32_t max_range;  // +0x300: From source piVar1[0x7f]
    uint32_t hits;  // +0x304: From source piVar1[0x84]
    uint32_t armor;  // +0x308: From source piVar1[0x85]
    uint32_t line_of_sight;  // +0x30C: From source piVar1[0x87]
    uint32_t science_LoS;  // +0x310: From source piVar1[0x88]
    
    // building-specific extracted properties (7 properties vs 4 for units)
    uint32_t town_hits;  // +0x314: From source piVar1[0xad]
    uint32_t plunder_amount;  // +0x318: From source piVar1[0xb5]
    /*
        0 = Food
        1 = Timber
        2 = Wealth
        3 = Knowledge
        4 = Metal
        5 = Oil
    */
    uint32_t plunder_good;  // +0x31C: From source piVar1[0xb6]
    uint32_t garrison_max;  // +0x320: From source piVar1[0xb2]
    uint32_t base_arrows;  // +0x324: From source piVar1[0xb3]
    uint32_t most_shots;  // +0x328: From source piVar1[0xb1] 
    uint32_t wonder_points;  // +0x32C: From source piVar1[0xb4]
};
#pragma pack(pop)
static_assert(sizeof(BuildingObjectDataChunk0x3B) == 0x330, "BuildingObjectDataChunk0x3B header size mismatch");
