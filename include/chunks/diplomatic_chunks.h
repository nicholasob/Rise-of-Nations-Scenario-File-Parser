#pragma once
#include "../base/byte_convertible.h"
#include "../common/common_types.h"
#include <cstdint>

#pragma pack(push, 1)
struct DiplomaticCountSubChunk0x36 : public ByteConvertible<DiplomaticCountSubChunk0x36> {
    uint32_t diplomatic_count;    //number of active diplomatic relations in the scenario
};
#pragma pack(pop)
static_assert(sizeof(DiplomaticCountSubChunk0x36) == 4, "DiplomaticCountSubChunk0x36 header size mismatch");

//Total: 520 bytes (0x208)
//Unsure about content! if correct that is...
struct DiplomaticEntryData {
	char16_t agreement_name[256];	// 512 bytes: agreement name/description
	uint32_t diplomatic_terms; // 4 bytes: treaty duration, trade values, military support
	uint32_t diplomatic_status;	//4 bytes: alliance level, agreement state, relationship type
};
static_assert(sizeof(DiplomaticEntryData) == 0x208, "DiplomaticEntryData header size mismatch");

#pragma pack(push, 1)
struct DiplomaticEntrySubChunk0x37 : public VariableLengthArrayChunk<DiplomaticEntrySubChunk0x37, DiplomaticEntryData> {
    std::vector<DiplomaticEntryData> diplomatic_entries;

    DiplomaticEntrySubChunk0x37() = default;

    DiplomaticEntrySubChunk0x37(const DiplomaticCountSubChunk0x36& header) {
        diplomatic_entries.resize(header.diplomatic_count);
    }

    DiplomaticEntrySubChunk0x37(const size_t& trigger_count) {
        diplomatic_entries.resize(trigger_count);
    }

    std::vector<DiplomaticEntryData>& get_container() override {
        return diplomatic_entries;
    }
    
    const std::vector<DiplomaticEntryData>& get_container() const override {
        return diplomatic_entries;
    }
};
#pragma pack(pop)
