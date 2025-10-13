#pragma once
#include "../base/byte_convertible.h"
#include "../common/common_types.h"
#include <cstdint>

#pragma pack(push, 1)
// Chunk 0x30: editor Order Group Data Structure  
// Total size: 520 bytes (0x208)
// this chunk stores organizational units defined by scenario designers for tactical formations, spawn groups, or design organization
struct EditorOrderGroupChunk0x30 : public ByteConvertible<EditorOrderGroupChunk0x30> {
    //contains group specific settings and properties
    uint32_t group_config;            // +0x00 (4 bytes)

    //Number of units in this editor order group (4 bytes)
    //Valid range: 0-128 units per group
    uint32_t units_in_group;            // +0x08 (4 bytes)

    // Unit ID array (512 bytes = 128 * 4 bytes)
    // Each entry is a 4 byte unit ID
    // Only first 'units_in_group' entries are valid & remaining entries MAY contain garbage data
    uint32_t unit_ids[128];
};
#pragma pack(pop)
static_assert(sizeof(EditorOrderGroupChunk0x30) == 0x208, "EditorOrderGroupChunk0x30 must be exactly 520 bytes");

#pragma pack(push, 1)
// Chunk 0x32: Editor Element Count Data Structure
// Total size: 4 bytes
// This chunk stores the count of editor elements in the scenario
// Eeditor elements include map annotations, waypoints, spawn points, trigger zones, and other design elements created during scenario editing
struct EditorElementCountChunk0x32 : public ByteConvertible<EditorElementCountChunk0x32> {
    // number of editor elements in the scenario (4 bytes)
    // this count determines how many 16-byte editor element records will follow in the subsequent chunk 0x33
    uint32_t element_count;         // +0x00 (4 bytes)
    
    //Total struct size: 4 bytes
};
#pragma pack(pop)
static_assert(sizeof(EditorElementCountChunk0x32) == 0x4, "EditorElementCountChunk0x32 must be exactly 4 bytes");

#pragma pack(push, 1)
// Chunk 0x33: Editor Element Data Structure
// Size: 16 bytes per element × element_count (from chunk 0x32)
// Chunk contains the actual editor element data for map annotations, waypoints, spawn points, trigger zones, and other design elements created during scenario editing
struct EditorElementDataStructure0x33 : public ByteConvertible<EditorElementDataStructure0x33> {
    // Element configuration/flags (4 bytes)
    // Contains element type, behavior flags, and other properties
    // From DAT_00ed66e4 in the linked list
    int32_t element_config;        // +0x00 (4 bytes)
    
    // Element type/properties (4 bytes) 
    // Defines what kind of editor element this is
    // From DAT_00ed66e0 in the linked list
    int32_t element_type;          // +0x04 (4 bytes)
    
    // element X coordinate (4 bytes)
    // Map position in game world coordinates
    // From DAT_00ed66d8 in the linked list
    uint32_t x_coordinate;          // +0x08 (4 bytes)
    
    // element Y coordinate (4 bytes)
    // Map position in game world coordinates  
    // From DAT_00ed66dc in the linked list
    uint32_t y_coordinate;          // +0x0C (4 bytes)
    
    //Total struct size: 16 bytes (0x10)
};
#pragma pack(pop)
static_assert(sizeof(EditorElementDataStructure0x33) == 0x10, "EditorElementDataStructure0x33 must be exactly 16 bytes");
