#pragma once
#include "../base/byte_convertible.h"
#include <cstdint>

#pragma pack(push, 1)
struct ResourceEntry {
    uint32_t resource_index;    // Resource type index (0, 1, 2, 3, 4, 5)
    uint32_t value;             // Resource value (might be XORed with 0x8221?)
};
#pragma pack(pop)
static_assert(sizeof(ResourceEntry) == 0x8, "ResourceEntry must be exactly 8 bytes");

#pragma pack(push, 1)
struct ResourceTypeCountChunk0x2d : public ByteConvertible<ResourceTypeCountChunk0x2d> {
    uint32_t resource_type_count; //number of resource types
};
#pragma pack(pop)
static_assert(sizeof(ResourceTypeCountChunk0x2d) == 0x4, "ResourceTypeCountChunk0x2d must be exactly 4 bytes");

#pragma pack(push, 1)
struct ResourceTypeChunk0x2e : public VariableLengthArrayChunk<ResourceTypeChunk0x2e, ResourceEntry> {
    std::vector<ResourceEntry> resource_entries;

    ResourceTypeChunk0x2e() = default;

    ResourceTypeChunk0x2e(const ResourceTypeCountChunk0x2d& header) {
        resource_entries.resize(header.resource_type_count);
    }

    ResourceTypeChunk0x2e(const size_t& count = 6) { //by default its set to size 6, since we have 6 resource types.
        resource_entries.resize(count);
    }

    //implementation of pure virtual functions
    std::vector<ResourceEntry>& get_container() override {
        return resource_entries;
    }
    
    const std::vector<ResourceEntry>& get_container() const override {
        return resource_entries;
    }
};
#pragma pack(pop)
