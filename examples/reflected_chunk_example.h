#pragma once
#include "../include/base/byte_convertible.h"
#include "../include/base/chunk_reflection.h"
#include "../include/common/common_types.h"
#include <cstdint>

/**
 * @brief Example of a reflected chunk structure
 *
 * This demonstrates how to add reflection metadata to chunk structs
 * so the GUI can automatically display their fields.
 */

#pragma pack(push, 1)
struct ReflectedTechTreeUnitTypeNameChunk0x56 : public ByteConvertible<ReflectedTechTreeUnitTypeNameChunk0x56> {
    // Actual struct fields
    char16_t unit_type_name[256];

    // Reflection metadata (defined AFTER the fields)
    BEGIN_FIELD_DESCRIPTORS(ReflectedTechTreeUnitTypeNameChunk0x56)
        DESCRIBE_FIELD(char16_t[256], unit_type_name, "Unit type name (UTF-16)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
static_assert(sizeof(ReflectedTechTreeUnitTypeNameChunk0x56) == 0x200,
              "ReflectedTechTreeUnitTypeNameChunk0x56 must be exactly 512 bytes");


/**
 * @brief Example with multiple fields
 */
#pragma pack(push, 1)
struct ReflectedFormationChunk0x53 : public ByteConvertible<ReflectedFormationChunk0x53> {
    uint32_t player_index;
    uint32_t formation_unit_id;
    int32_t ll_prev_group_member;
    int32_t ll_next_group_member;
    uint32_t field_0x10;
    uint32_t field_0x14;
    uint32_t field_0x18;
    uint32_t formation_flags;
    uint16_t formation_type;
    uint8_t formation_behavior1;
    uint8_t formation_behavior2;
    uint8_t formation_special_flag;
    uint8_t padding_byte;
    char16_t formation_unit_name[257];
    uint32_t position_x;
    uint32_t position_y;
    uint32_t damage_taken;
    uint32_t field_after_name1;
    uint32_t field_after_name2;
    uint32_t group_id;
    uint32_t unit_index_in_group;

    // Reflection metadata
    BEGIN_FIELD_DESCRIPTORS(ReflectedFormationChunk0x53)
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
static_assert(sizeof(ReflectedFormationChunk0x53) == 0x244,
              "ReflectedFormationChunk0x53 must be exactly 580 bytes");


/**
 * HOW TO USE IN GUI chunk_metadata.cpp:
 *
 * Instead of manually writing:
 *   ChunkInfo techTreeUnitNames(...);
 *   techTreeUnitNames.fields.push_back(FieldInfo(...));
 *   registerChunk(techTreeUnitNames);
 *
 * Just write:
 *   registerReflectedChunk<ReflectedTechTreeUnitTypeNameChunk0x56>(
 *       ChunkType::TECH_TREE_UNIT_TYPE_NAMES,
 *       "TECH_TREE_UNIT_TYPE_NAMES",
 *       "Tech Tree",
 *       512,
 *       colorMetadata
 *   );
 *
 * The fields are extracted automatically from the struct!
 */
