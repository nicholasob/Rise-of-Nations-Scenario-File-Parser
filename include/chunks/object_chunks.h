#pragma once
#include "../base/byte_convertible.h"
#include "../common/common_types.h"
#include <cstdint>

// Unit data payload (796 bytes total)
#pragma pack(push, 1)
struct UnitsObjectDataChunk0x3A : public ByteConvertible<UnitsObjectDataChunk0x3A> {
    //basic unit properties (written by FUN_00667ee0) & based on editor trial and error
    uint32_t job_time;     // +0x00: From source offset 0x08
    uint32_t property_0x04;     // +0x04: From source offset 0x04  
    uint32_t property_0x10;     // +0x08: From source offset 0x10
    
    // string area 1 (variable length, max 512 bytes)
    // +0x0C to +0x20B: unit name/description string (wide char)
    char16_t original_unit_name[256]; // +0x0C: Max 255 chars + null terminator
    
    // string area 2 (variable length, max 200 bytes)  
    // +0x20C to +0x2D3: Secondary string data (wide char)
    char16_t custom_unit_name[100]; // +0x20C: Max 99 chars + null terminator
    
    // Fixed position unit properties (written by FUN_00667ee0) & based on editor trial and error
    uint32_t required_1;        // +0x2D4: param_2[0xb5] = source[0x30]
    uint32_t required_2;        // +0x2D8: param_2[0xb6] = source[0x34] 
    uint32_t required_3;        // +0x2DC: param_2[0xb7] = source[0x38]
    //*ALL COST VARIABLES ARE/MUST BE MULTIPLES OF 10*//
    //*SO IF VALUE IS 8, ACUTUAL COST IN GAME IS 80!!!!!!*//
    uint32_t food_cost;      // +0x2E0: param_2[0xb8] = source[0x18]
    uint32_t timber_cost;      // +0x2E4: param_2[0xb9] = source[0x1c]
    uint32_t wealth_cost;      // +0x2E8: param_2[0xba] = source[0x20]
    uint32_t knowledge_cost;      // +0x2EC: param_2[0xbb] = source[0x24]
    uint32_t metal_cost;      // +0x2F0: param_2[0xbc] = source[0x28]
    uint32_t oil_cost;      // +0x2F4: param_2[0xbd] = source[0x2c]
    
    // Additional unit properties (written by FUN_0065fac0) & based on editor trial and error
    uint32_t attack;     // +0x2F8: From source offset 0x1e8
    uint32_t min_range;     // +0x2FC: From source offset 0x1f8
    uint32_t max_range;     // +0x300: From source offset 0x1fc
    uint32_t hits;     // +0x304: From source offset 0x210
    uint32_t armor;     // +0x308: From source offset 0x214
    uint32_t line_of_sight;     // +0x30C: From source offset 0x21c
    uint32_t science_LoS;     // +0x310: From source offset 0x220
    
    // Extracted unit properties (copied from high offsets)
    uint32_t moves;  // +0x314: From source offset 0x2c0
    //decode182Scaled
    uint32_t turn_speed;  // +0x318: From source offset 0x2c4
    uint32_t craft;  // +0x31C: From source offset 0x2ec
    uint32_t control_cost;  // +0x320: From source offset 0x2f0
};
#pragma pack(pop)
static_assert(sizeof(UnitsObjectDataChunk0x3A) == 0x324, "UnitsObjectDataChunk0x3A header size mismatch");

// Resource data payload (760 bytes exactly - 0x2F8)
#pragma pack(push, 1)
struct ResourceObjectDataChunk0x3C : public ByteConvertible<ResourceObjectDataChunk0x3C> {
    // basic resource properties (written by FUN_00667ee0)
    uint32_t property_0x08;     // +0x00: From source offset 0x08
    uint32_t property_0x04;     // +0x04: From source offset 0x04  
    uint32_t property_0x10;     // +0x08: From source offset 0x10
    
    // String area 1 (variable length, max 512 bytes)
    char16_t string_data_1[256]; // +0x0C: Resource name/description (max 255 chars + null)
    
    // String area 2 (variable length, max 200 bytes)  
    char16_t string_data_2[100]; // +0x20C: Secondary string data (max 99 chars + null)
    
    // Fixed position resource properties (written by FUN_00667ee0)
    uint32_t resource_pos_x;    // +0x2D4: param_2[0xb5] = source[0x30]
    uint32_t resource_pos_y;    // +0x2D8: param_2[0xb6] = source[0x34] 
    uint32_t resource_pos_z;    // +0x2DC: param_2[0xb7] = source[0x38]
    uint32_t resource_prop_18;  // +0x2E0: param_2[0xb8] = source[0x18]
    uint32_t resource_prop_1c;  // +0x2E4: param_2[0xb9] = source[0x1c]
    uint32_t resource_prop_20;  // +0x2E8: param_2[0xba] = source[0x20]
    uint32_t resource_prop_24;  // +0x2EC: param_2[0xbb] = source[0x24]
    uint32_t resource_prop_28;  // +0x2F0: param_2[0xbc] = source[0x28]
    uint32_t resource_prop_2c;  // +0x2F4: param_2[0xbd] = source[0x2c]
    
    // NO additional extracted properties (unlike units/buildings)
    // Structure ends at +0x2F8 (760 bytes total)
};
#pragma pack(pop)
static_assert(sizeof(ResourceObjectDataChunk0x3C) == 0x2F8, "ResourceObjectDataChunk0x3C header size mismatch");

//single object group entry with variable number of object IDs
struct ObjectGroupEntry : public ByteConvertible<ObjectGroupEntry> {
    uint32_t object_count;              //number of objects in this group
    std::vector<uint32_t> object_ids;   //variable length array of object IDs

    ObjectGroupEntry() : object_count(0) {}
    
    ObjectGroupEntry(uint32_t count) : object_count(count) {
        object_ids.resize(count);
    }

    static ObjectGroupEntry from_bytes(const std::vector<std::byte>& bytes, size_t& offset) {
        if (offset + sizeof(uint32_t) > bytes.size()) {
            throw std::runtime_error("Insufficient bytes for object count");
        }
        
        ObjectGroupEntry entry;
        std::memcpy(&entry.object_count, bytes.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        
        if (offset + entry.object_count * sizeof(uint32_t) > bytes.size()) {
            throw std::runtime_error("Insufficient bytes for object IDs");
        }
        
        entry.object_ids.resize(entry.object_count);
        for (uint32_t i = 0; i < entry.object_count; ++i) {
            std::memcpy(&entry.object_ids[i], bytes.data() + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
        }
        
        return entry;
    }

    std::vector<std::byte> to_bytes() const {
        std::vector<std::byte> result;
        result.reserve(sizeof(uint32_t) + object_ids.size() * sizeof(uint32_t));
        
        //add object count
        const std::byte* count_bytes = reinterpret_cast<const std::byte*>(&object_count);
        result.insert(result.end(), count_bytes, count_bytes + sizeof(uint32_t));
        
        //add object IDs
        for (const uint32_t& id : object_ids) {
            const std::byte* id_bytes = reinterpret_cast<const std::byte*>(&id);
            result.insert(result.end(), id_bytes, id_bytes + sizeof(uint32_t));
        }
        
        return result;
    }

    size_t byte_size() const {
        return sizeof(uint32_t) + object_ids.size() * sizeof(uint32_t);
    }
};

#pragma pack(push, 1)
struct TileCountChunk0x64 : public ByteConvertible<TileCountChunk0x64> {
    uint32_t total_tiles;
};
#pragma pack(pop)
static_assert(sizeof(TileCountChunk0x64) == 0x4, "TileCountChunk0x64 must be exactly 4 bytes");

#pragma pack(push, 1)
// Chunk 0x5A: Object Groups Data - Variable length groups structure
struct ObjectGroupsChunk0x5A : public ByteConvertible<ObjectGroupsChunk0x5A> {
    uint32_t group_count;                           //number of object groups
    std::vector<ObjectGroupEntry> object_groups;   //variable length array of groups

    ObjectGroupsChunk0x5A() : group_count(0) {}
    
    ObjectGroupsChunk0x5A(uint32_t count) : group_count(count) {
        object_groups.reserve(count);
    }

    static ObjectGroupsChunk0x5A from_bytes(const std::vector<std::byte>& bytes) {
        if (bytes.size() < sizeof(uint32_t)) {
            throw std::runtime_error("Insufficient bytes for group count");
        }
        
        ObjectGroupsChunk0x5A chunk;
        size_t offset = 0;
        
        //read group count
        std::memcpy(&chunk.group_count, bytes.data(), sizeof(uint32_t));
        offset += sizeof(uint32_t);
        
        //read each group
        chunk.object_groups.reserve(chunk.group_count);
        for (uint32_t i = 0; i < chunk.group_count; ++i) {
            ObjectGroupEntry group = ObjectGroupEntry::from_bytes(bytes, offset);
            chunk.object_groups.push_back(std::move(group));
        }
        
        return chunk;
    }

    std::vector<std::byte> to_bytes() const {
        std::vector<std::byte> result;
        
        //add group count
        const std::byte* count_bytes = reinterpret_cast<const std::byte*>(&group_count);
        result.insert(result.end(), count_bytes, count_bytes + sizeof(uint32_t));
        
        //add each group
        for (const ObjectGroupEntry& group : object_groups) {
            std::vector<std::byte> group_bytes = group.to_bytes();
            result.insert(result.end(), group_bytes.begin(), group_bytes.end());
        }
        
        return result;
    }

    size_t byte_size() const {
        size_t total = sizeof(uint32_t); //group_count
        for (const ObjectGroupEntry& group : object_groups) {
            total += group.byte_size();
        }
        return total;
    }

    //helper methods
    void add_group(const ObjectGroupEntry& group) {
        object_groups.push_back(group);
        group_count = static_cast<uint32_t>(object_groups.size());
    }

    void add_group(const std::vector<uint32_t>& object_ids) {
        ObjectGroupEntry group(static_cast<uint32_t>(object_ids.size()));
        group.object_ids = object_ids;
        add_group(group);
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
// Chunk 0x5b: Map Links Data - Variable length array of link IDs
struct MapLinksChunk0x5b : public VariableLengthArrayChunk<MapLinksChunk0x5b, uint16_t> {
    std::vector<uint16_t> link_ids;     //variable length array of link identifiers
                                        //each link represents connection between map regions
                                        //or references to linked objects

    MapLinksChunk0x5b() = default;

    MapLinksChunk0x5b(const size_t& link_count) {
        link_ids.resize(link_count);
    }

    //implementation of pure virtual functions from VariableLengthArrayChunk
    std::vector<uint16_t>& get_container() override {
        return link_ids;
    }
    
    const std::vector<uint16_t>& get_container() const override {
        return link_ids;
    }
};
#pragma pack(pop)
