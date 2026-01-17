#pragma once
#include "../base/byte_convertible.h"
#include "../base/chunk_reflection.h"
#include "../common/common_types.h"
#include <cstdint>

#pragma pack(push, 1)
struct FormationChunk0x53 : public ByteConvertible<FormationChunk0x53> {
    //based on exact memory layout from WriteFormationChunkData starting at local_270
    //total size: 0x244 (580) bytes as confirmed by memcpy operation
    //first 48 bytes seems to be various formation metadata fields
    uint32_t player_index;                    // +0x000: local_270 - the player owning this formation (0-7)
    uint32_t formation_unit_id;               // +0x004: local_26c - formation unit pointer/ID
    int32_t ll_prev_group_member;             // +0x008: local_268 - formation group previous member, linked list (-1 if none)
    int32_t ll_next_group_member;             // +0x00C: local_264 - formation group next member, linked list (-1 if none)

    uint32_t field_0x10;                      // +0x010: local_260 - unknown field
    uint32_t field_0x14;                      // +0x014: local_25c - unknown field
    uint32_t field_0x18;                      // +0x018: local_258 - unknown field
    uint32_t formation_flags;                 // +0x01C: local_254 - formation visibility/state flags
                                              //   bit 0x1: unit visible
                                              //   bit 0x2: unit combat active
                                              //   bit 0x4: formation in combat
                                              //   bit 0x8: follower unit (not leader)
                                              //   bit 0x10: formation special state

    uint16_t formation_type;                  // +0x020: local_250 - formation type identifier
    uint8_t formation_behavior1;              // +0x022: local_24e - formation behavior flags
    uint8_t formation_behavior2;              // +0x023: local_24d - formation behavior flags
    uint8_t formation_special_flag;           // +0x024: local_24c - special formation flag (0 normally)
    uint8_t padding_byte;                     // +0x025: padding to align name field (i guess)
    //formation unit name is 514 bytes (or 257 UTF-16 code units)
    char16_t formation_unit_name[257];        // +0x030: unit name in formation (a null-terminated UTF-16)

    //remaining 28 bytes to reach exactly 0x244 total
    //Map layout:
    //  Top Middle: [0][0]
    //  Middle left: [0][MAP_HEIGHT]
    //  Middle Right: [MAP_WIDTH][0]
    //  Bottom Bottom: [MAP_WIDTH][MAP_HEIGHT]
    uint32_t position_x;                      // +0x228: additional field
    uint32_t position_y;                      // +0x22C: additional field
    //for instance, if unit has 24/40, that means this variable is set to 16 since (40-16=24)
    //max_health - current_health
    uint32_t damage_taken;                    // +0x230: additional field
    uint32_t field_after_name1;               // +0x234: additional field
    uint32_t field_after_name2;               // +0x238: additional field

    //if null, then it doesn't belong to a group (i.e its a single unit, will only select one instance)
    uint32_t group_id;          // +0x23C: ID of the group the unit belongs to (-1 if none)
    uint32_t unit_index_in_group; // +0x240: Index or ID of the unit within the group (-1 if none) - ignore value if group_id=-1

    BEGIN_FIELD_DESCRIPTORS(FormationChunk0x53)
        DESCRIBE_FIELD(uint32_t, player_index, "Player owning formation (0-7)")
        DESCRIBE_FIELD(uint32_t, formation_unit_id, "Formation unit pointer/ID")
        DESCRIBE_FIELD(int32_t, ll_prev_group_member, "Linked list prev (-1 if none)")
        DESCRIBE_FIELD(int32_t, ll_next_group_member, "Linked list next (-1 if none)")
        DESCRIBE_FIELD(uint32_t, field_0x10, "Field at offset 0x10")
        DESCRIBE_FIELD(uint32_t, field_0x14, "Field at offset 0x14")
        DESCRIBE_FIELD(uint32_t, field_0x18, "Field at offset 0x18")
        DESCRIBE_FIELD(uint32_t, formation_flags, "Visibility/state flags")
        DESCRIBE_FIELD(uint16_t, formation_type, "Formation type identifier")
        DESCRIBE_FIELD(uint8_t, formation_behavior1, "Formation behavior 1")
        DESCRIBE_FIELD(uint8_t, formation_behavior2, "Formation behavior 2")
        DESCRIBE_FIELD(uint8_t, formation_special_flag, "Special flag")
        DESCRIBE_FIELD(uint8_t, padding_byte, "Padding")
        DESCRIBE_FIELD(char16_t[257], formation_unit_name, "Unit name (UTF-16)")
        DESCRIBE_FIELD(uint32_t, position_x, "X position")
        DESCRIBE_FIELD(uint32_t, position_y, "Y position")
        DESCRIBE_FIELD(uint32_t, damage_taken, "max_health - current_health")
        DESCRIBE_FIELD(uint32_t, field_after_name1, "Field after name 1")
        DESCRIBE_FIELD(uint32_t, field_after_name2, "Field after name 2")
        DESCRIBE_FIELD(uint32_t, group_id, "Group ID (-1 if none)")
        DESCRIBE_FIELD(uint32_t, unit_index_in_group, "Index in group (-1 if none)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
//verify struct size matches expected chunk data size
static_assert(sizeof(FormationChunk0x53) == 0x244, "FormationChunk0x53 must be exactly 580 bytes");

#pragma pack(push, 1)
struct TechTreeUnitTypeCountChunk0x27 : public ByteConvertible<TechTreeUnitTypeCountChunk0x27> {
    uint32_t unit_type_count; //number of active tech tree unit types

    BEGIN_FIELD_DESCRIPTORS(TechTreeUnitTypeCountChunk0x27)
        DESCRIBE_FIELD(uint32_t, unit_type_count, "Number of unit types")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
static_assert(sizeof(TechTreeUnitTypeCountChunk0x27) == 0x4, "TechTreeUnitTypeCountChunk0x27 must be exactly 4 bytes");

#pragma pack(push, 1)
struct TechTreeUnitTypeNameChunk0x56 : public VariableLengthArrayChunk<TechTreeUnitTypeNameChunk0x56, TechTreeTypeName> {
    std::vector<TechTreeTypeName> unit_names;

    TechTreeUnitTypeNameChunk0x56() = default;

    TechTreeUnitTypeNameChunk0x56(const TechTreeUnitTypeCountChunk0x27& header) {
        unit_names.resize(header.unit_type_count);
    }

    TechTreeUnitTypeNameChunk0x56(const size_t& unit_count) {
        unit_names.resize(unit_count);
    }

    //implementation of pure virtual functions
    std::vector<TechTreeTypeName>& get_container() override {
        return unit_names;
    }
    
    const std::vector<TechTreeTypeName>& get_container() const override {
        return unit_names;
    }
};
#pragma pack(pop)
