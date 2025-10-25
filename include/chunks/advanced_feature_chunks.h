#pragma once
#include "../base/byte_convertible.h"
#include "../base/chunk_base.h"
#include "../common/common_types.h"
#include <cstdint>
#include <cstddef>
#include <vector>
#include <stdexcept>
#include <cstring>


class CoordinateGroup{
public:
    uint32_t NumberOfCoordinates;
    std::vector<Coordinate> Coordinates;

    CoordinateGroup(uint32_t numberOfCoordinates){
        this->NumberOfCoordinates = numberOfCoordinates;
        Coordinates.reserve(numberOfCoordinates);
    }
};

class SpatialGroup{
private:
public:
    uint32_t NumberOfCoordinateGroups;
    std::vector<CoordinateGroup> CoordinateGroup;

    SpatialGroup(uint32_t numberOfCoordinateGroups){
        this->NumberOfCoordinateGroups = numberOfCoordinateGroups;
        CoordinateGroup.reserve(numberOfCoordinateGroups);
    }
};

#pragma pack(push, 1)
struct AdvancedFeature1MountainFeaturesChunk0x46 : public ByteConvertible<AdvancedFeature1MountainFeaturesChunk0x46> {
    uint32_t feature_count; //number of mountain features that follow in chunk 0x47
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AdvancedFeature1MountainFeatureEntry : public ByteConvertible<AdvancedFeature1MountainFeatureEntry> {
    uint32_t feature_id; //Mountain/terrain feature ID from global array
    uint8_t mountain_flag1; //Mountain-related flag from FUN_0089cf90
    uint8_t mountain_flag2; //Mountain-related flag from FUN_0089d080
    uint16_t padding;   //alignment paddin
    uint32_t feature_properties;    //Feature properties from mountain data structure (+0x44 offset)
};
#pragma pack(pop)
static_assert(sizeof(AdvancedFeature1MountainFeatureEntry) == 0xc, "MountainFeatureEntry must be exactly 12 bytes");

#pragma pack(push, 1)
//Chunk 0x12: Terrain Type Data
struct AdvancedFeature1MountainFeatureChunk0x47 : public VariableLengthArrayChunk<AdvancedFeature1MountainFeatureChunk0x47, AdvancedFeature1MountainFeatureEntry> {
    std::vector<AdvancedFeature1MountainFeatureEntry> feature_entries;

    AdvancedFeature1MountainFeatureChunk0x47() = default;

    AdvancedFeature1MountainFeatureChunk0x47(const AdvancedFeature1MountainFeaturesChunk0x46& header) {
        feature_entries.resize(header.feature_count);
    }

    AdvancedFeature1MountainFeatureChunk0x47(const size_t& unit_count) {
        feature_entries.resize(unit_count);
    }

    std::vector<AdvancedFeature1MountainFeatureEntry>& get_container() override {
        return feature_entries;
    }
    
    const std::vector<AdvancedFeature1MountainFeatureEntry>& get_container() const override {
        return feature_entries;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
// Chunk 0x5F: Coordinate Entry
//individual coordinate/position entry (8 bytes data)
struct AdvancedFeature2CoordinateArrayChunk0x5F : public ByteConvertible<AdvancedFeature2CoordinateArrayChunk0x5F> {
    uint32_t x_processed;   //X coordinate after the lookup table processing
    uint32_t y_processed;   //Y coordinate after the lookup table processing
    
    AdvancedFeature2CoordinateArrayChunk0x5F(uint32_t x = 0, uint32_t y = 0) : x_processed(x), y_processed(y) {}
};
#pragma pack(pop)
static_assert(sizeof(AdvancedFeature2CoordinateArrayChunk0x5F) == 0x8, "CoordinateEntry must be exactly 8 bytes");

// Chunk 0x5D: Spatial Group Count
//this is the main chunk that contains the number of spatial groups
#pragma pack(push, 1)
struct AdvancedFeature2SpatialGroupCountChunk0x5D : public ByteConvertible<AdvancedFeature2SpatialGroupCountChunk0x5D> {
    uint32_t group_count;   // number of spatial groups (from DAT_00c11c14), each group will have a corresponding 0x5e sub-chunk
};
#pragma pack(pop)
static_assert(sizeof(AdvancedFeature2SpatialGroupCountChunk0x5D) == 0x4, "SpatialGroupCountChunk0x5D must be exactly 4 bytes");

// Chunk 0x5E: Group Coordinate Count
// this chunk contains the count of coordinates that will follow in 0x5f sub-chunks
#pragma pack(push, 1)
struct AdvancedFeature2GroupCoordinateCountChunk0x5E : public ByteConvertible<AdvancedFeature2GroupCoordinateCountChunk0x5E> {
    uint32_t coordinate_count;  //number of coordinate entries in this group (from offset 0x68 in group structure)
};
#pragma pack(pop)
static_assert(sizeof(AdvancedFeature2GroupCoordinateCountChunk0x5E) == 0x4, "GroupCoordinateCountChunk0x5E must be exactly 4 bytes");

// Sub-chunk 0x61 - Contains count of 0x62 entries
#pragma pack(push,1)
struct AdvancedFeature3Chunk0x61 : public ByteConvertible<AdvancedFeature3Chunk0x61> {
    int32_t entry_count;  //number of Chunk62Data entries that follow
};
#pragma pack(pop)

// Sub-chunk 0x62 - Contains coordinate/position data (repeats entry_count times)
struct AdvancedFeature3Chunk62Data {
    union {
        struct {
            float field_0x40;     // From source object offset 0x40 
            float field_0x44;     // From source object offset 0x44
            float field_0x38;     // From source object offset 0x38  
            float field_0x3c;     // From source object offset 0x3c
        };
        struct {
            int32_t int_0x40;     // Alternative interpretation as integers
            int32_t int_0x44;
            int32_t int_0x38;
            int32_t int_0x3c;
        };
        uint32_t raw_data[4];     // Raw 32-bit values
    };
};

#pragma pack(push,1)
struct AdvancedFeature3CoordinateArrayChunk0x62 : public VariableLengthArrayChunk<AdvancedFeature3CoordinateArrayChunk0x62, AdvancedFeature3Chunk62Data> {
    std::vector<AdvancedFeature3Chunk62Data> coordinate_entries;

    AdvancedFeature3CoordinateArrayChunk0x62() = default;

    AdvancedFeature3CoordinateArrayChunk0x62(const AdvancedFeature3Chunk0x61& header) {
        coordinate_entries.resize(header.entry_count);
    }

    AdvancedFeature3CoordinateArrayChunk0x62(const size_t& coordinate_count) {
        coordinate_entries.resize(coordinate_count);
    }

    std::vector<AdvancedFeature3Chunk62Data>& get_container() override {
        return coordinate_entries;
    }
    
    const std::vector<AdvancedFeature3Chunk62Data>& get_container() const override {
        return coordinate_entries;
    }
};
#pragma pack(pop)

// Sub-chunk 0x4A - contains count of waypoint entries
#pragma pack(push,1)
struct AdvancedFeature4Chunk0x4a : public ByteConvertible<AdvancedFeature4Chunk0x4a> {
    int32_t waypoint_count;  //number of waypoints in the following sub-chunk
};
#pragma pack(pop)

//individual waypoint entry structure
struct WaypointEntryChunk0x4b {
    int32_t x_coordinate; // X position (converted from float to int)
    int32_t y_coordinate; // Y position (converted from float to int)
    uint32_t flags_and_id; // contains waypoint ID and status flags
};

#pragma pack(push,1)
struct AdvancedFeature4Chunk0x4b : public VariableLengthArrayChunk<AdvancedFeature4Chunk0x4b, WaypointEntryChunk0x4b> {
    std::vector<WaypointEntryChunk0x4b> waypoints_count;

    AdvancedFeature4Chunk0x4b() = default;

    AdvancedFeature4Chunk0x4b(const AdvancedFeature4Chunk0x4a& header) {
        waypoints_count.resize(header.waypoint_count);
    }

    AdvancedFeature4Chunk0x4b(const size_t& coordinate_count) {
        waypoints_count.resize(coordinate_count);
    }

    std::vector<WaypointEntryChunk0x4b>& get_container() override {
        return waypoints_count;
    }
    
    const std::vector<WaypointEntryChunk0x4b>& get_container() const override {
        return waypoints_count;
    }
};
#pragma pack(pop)

// Variable Data Chunk - Complex variable-size chunk with conditional fields
// Chunk 0x08: Variable Data Chunk - Complex variable-size chunk with conditional fields
// this chunk requires special parsing due to its variable structure
#pragma pack(push, 1)
struct VariableDataChunk0x08 {
    uint32_t primary_value;         // always present: First 4-byte value
    bool has_flags_field;           // runtime-determined: Whether flags field is present
    uint32_t flags_optional;        // Conditional: Only present if has_flags_field is true
    uint32_t element_count;         // Always present: Number of 16-bit elements  
    std::vector<uint16_t> elements; // variable length: element_count elements
    
    //this structure cannot be used directly with ByteConvertible due to variable size
    // Static parsing method to be used  instead of from_bytes
    static VariableDataChunk0x08 parse_from_chunk(const Chunk& chunk) {
        VariableDataChunk0x08 result;
        
        if (chunk.data.size() < 8) { // Minimum: primary_value + element_count
            throw std::runtime_error("Chunk 0x08 too small for minimum structure");
        }
        
        size_t offset = 0;
        
        //read primary value (always present)
        std::memcpy(&result.primary_value, chunk.data.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        
        //calculate remaining data size
        size_t remaining_size = chunk.data.size() - offset;
        
        //read potential element count (could be at different positions)
        uint32_t potential_count;
        std::memcpy(&potential_count, chunk.data.data() + offset, sizeof(uint32_t));
        
        //determine if flags field is present based on size constraints
        size_t expected_size_without_flags = sizeof(uint32_t) + (potential_count * sizeof(uint16_t));
        size_t expected_size_with_flags = sizeof(uint32_t) + sizeof(uint32_t) + (potential_count * sizeof(uint16_t));
        
        if (remaining_size == expected_size_without_flags) {
            //no flags field present
            result.has_flags_field = false;
            result.flags_optional = 0;
            result.element_count = potential_count;
        } else if (remaining_size == expected_size_with_flags) {
            //flags field is present
            result.has_flags_field = true;
            result.flags_optional = potential_count;
            offset += sizeof(uint32_t);
            
            //read the actual element count
            std::memcpy(&result.element_count, chunk.data.data() + offset, sizeof(uint32_t));
        } else {
            throw std::runtime_error("Chunk 0x08 size doesn't match expected patterns");
        }
        
        offset += sizeof(uint32_t);
        
        //read elements if any
        result.elements.resize(result.element_count);
        if (result.element_count > 0 && offset + (result.element_count * sizeof(uint16_t)) <= chunk.data.size()) {
            for (uint32_t i = 0; i < result.element_count; ++i) {
                std::memcpy(&result.elements[i], chunk.data.data() + offset, sizeof(uint16_t));
                offset += sizeof(uint16_t);
            }
        }
        
        return result;
    }
    
    //convert back to bytes for serialization
    std::vector<uint8_t> to_bytes() const {
        std::vector<uint8_t> result;
        
        //add primary value
        const uint8_t* primary_bytes = reinterpret_cast<const uint8_t*>(&primary_value);
        result.insert(result.end(), primary_bytes, primary_bytes + sizeof(uint32_t));
        
        //add flags if present
        if (has_flags_field) {
            const uint8_t* flags_bytes = reinterpret_cast<const uint8_t*>(&flags_optional);
            result.insert(result.end(), flags_bytes, flags_bytes + sizeof(uint32_t));
        }
        
        //add element count
        const uint8_t* count_bytes = reinterpret_cast<const uint8_t*>(&element_count);
        result.insert(result.end(), count_bytes, count_bytes + sizeof(uint32_t));
        
        //add elements
        for (const uint16_t& element : elements) {
            const uint8_t* element_bytes = reinterpret_cast<const uint8_t*>(&element);
            result.insert(result.end(), element_bytes, element_bytes + sizeof(uint16_t));
        }
        
        return result;
    }
    
    size_t calculate_size() const {
        return sizeof(uint32_t) +                           // primary_value
               (has_flags_field ? sizeof(uint32_t) : 0) +   // flags_optional
               sizeof(uint32_t) +                           // element_count
               (elements.size() * sizeof(uint16_t));        // elements
    }
};
#pragma pack(pop)

//helper structure for string array descriptor analysis (for understanding the source data)
#pragma pack(push, 1)
struct StringArrayDescriptor {
    void*    data_ptr;          // Pointer to data (can be direct or indirect)
    uint16_t padding_or_field;  // Unknown field
    uint16_t offset;            // Offset within the data (multiplied by 2)
    uint16_t count;             // Number of 16-bit elements
    uint8_t  flags;             // Bit 0 = direct/indirect pointer flag
    uint8_t  padding;           // Padding byte
};
#pragma pack(pop)
