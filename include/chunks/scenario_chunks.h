#pragma once
#include "../base/byte_convertible.h"
#include "../common/common_types.h"
#include "../common/encryption.h"
#include <cstdint>

#pragma pack(push, 1)
struct ScenarioHeader0x6C : public ByteConvertible<ScenarioHeader0x6C> {
    uint32_t version;             // local_260 - possibly number of chunks or version
    uint32_t magic;               // local_25c - always 0xf00dcafe

    char16_t scenario_name[100];  // local_258 - 200 bytes (100 wchar_t) - wide-char scenario name/desc

    uint32_t player_setting1;     // local_190 = PTR_DAT_00c061ec[0x24]
    uint32_t player_setting2;     // local_18c = PTR_DAT_00c061ec[0x2a]
    uint32_t player_setting3;     // local_188 = PTR_DAT_00c061ec[0x25]
    uint32_t player_setting4;     // local_184 = PTR_DAT_00c061ec[0x26]

    uint32_t global_setting;      // local_180 = *(PTR_DAT_00c06188 + 0x7c)

    uint32_t game_flags;          // local_17c — various game mode flags
                                  // plus:
                                  // + |= 4 if (PTR_DAT_00c061ec[0x20] & 8)
                                  // + |= 8 if (PTR_DAT_00c061ec[0x20] & 4)
                                  // + |= 2 if (some global != 0)

    uint32_t ai_difficulty;       // local_178 = PTR_DAT_00c061ec[0x29]
    uint32_t victory_condition;   // local_174 = PTR_DAT_00c061ec[0x34]
    uint32_t map_type;            // local_170 = PTR_DAT_00c061ec[0x36]
    uint32_t setting1;            // local_16c = PTR_DAT_00c061ec[0x30]
    uint32_t setting2;            // local_168 = PTR_DAT_00c061ec[0x37]
    uint32_t setting3;            // local_164 = PTR_DAT_00c061ec[0x38]
    uint32_t setting4;            // local_160 = PTR_DAT_00c061ec[0x3a]
    uint32_t setting5;            // local_15c = PTR_DAT_00c061ec[0x3c]
    uint32_t setting6;            // local_158 = PTR_DAT_00c061ec[0x39]
    uint32_t setting7;            // local_154 = PTR_DAT_00c061ec[0x3b]
    uint32_t setting8;            // local_150 = PTR_DAT_00c061ec[0x3e]
    uint32_t player_setting5;     // local_14c = PTR_DAT_00c061ec[0x32]

    //global blob 1 (DAT_00cc22e0 - 00cc22ec)
    uint32_t global1_a;           // local_148
    uint32_t global1_b;           // uStack_144
    uint32_t global1_c;           // uStack_140
    uint32_t global1_d;           // uStack_13c

    //global blob 2 (DAT_00cc22f0 - 00cc22fc)
    uint32_t global2_a;           // local_138
    uint32_t global2_b;           // uStack_134
    uint32_t global2_c;           // uStack_130
    uint32_t global2_d;           // uStack_12c

    //global blob 3 (DAT_00cc0300 - 00cc030c)
    uint32_t global3_a;           // local_128
    uint32_t global3_b;           // uStack_124
    uint32_t global3_c;           // uStack_120
    uint32_t global3_d;           // uStack_11c

    //global blob 4 (DAT_00cc0310 - 00cc031c)
    uint32_t global4_a;           // local_118
    uint32_t global4_b;           // uStack_114
    uint32_t global4_c;           // uStack_110
    uint32_t global4_d;           // uStack_10c

    //global blob 5 (DAT_00cc21c0 - 00cc21cc)
    uint32_t global5_a;           // local_108
    uint32_t global5_b;           // uStack_104
    uint32_t global5_c;           // uStack_100
    uint32_t global5_d;           // uStack_fc

    //global blob 6 (DAT_00cc21d0 - 00cc21dc)
    uint32_t global6_a;           // local_f8
    uint32_t global6_b;           // uStack_f4
    uint32_t global6_c;           // uStack_f0
    uint32_t global6_d;           // uStack_ec

    uint32_t some_setting;        // local_e8 = *(PTR_DAT_00c06188 + 0x34)
    uint32_t extra_flag;          // local_e4 = PTR_DAT_00c061ec[0x31]
};
#pragma pack(pop)
static_assert(sizeof(ScenarioHeader0x6C) == 384, "ScenarioHeader must be exactly 384 bytes");

//individual trigger data entry (520 bytes each)
struct TriggerDataEntry0x3 {
    char16_t trigger_name[256];   // 512 bytes - trigger name/description
    uint32_t encrypted_param1;    // 4 bytes - conditions/timing (is XOR encrypted?)
    uint32_t encrypted_param2;    // 4 bytes - actions/targets (is XOR encrypted?)
    
    //helper methods to decrypt parameters
    uint32_t get_param1() const { return encrypted_param1 ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    uint32_t get_param2() const { return encrypted_param2 ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    void set_param1(uint32_t value) { encrypted_param1 = value ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    void set_param2(uint32_t value) { encrypted_param2 = value ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
};
static_assert(sizeof(TriggerDataEntry0x3) == 520, "TriggerDataEntry0x3 must be exactly 520 bytes");

//sub-chunk 0x15: Trigger count
#pragma pack(push, 1)
struct TriggerCountSubChunk0x15 : public ByteConvertible<TriggerCountSubChunk0x15> {
    uint32_t active_count;    //number of active triggers in the scenario
};
#pragma pack(pop)
static_assert(sizeof(TriggerCountSubChunk0x15) == 4, "TriggerCountSubChunk header size mismatch");

#pragma pack(push,1)
struct TriggerDataSubChunk0x16 : public VariableLengthArrayChunk<TriggerDataSubChunk0x16, TriggerDataEntry0x3> {
    std::vector<TriggerDataEntry0x3> triggers;

    TriggerDataSubChunk0x16() = default;

    TriggerDataSubChunk0x16(const TriggerCountSubChunk0x15& header) {
        triggers.resize(header.active_count);
    }

    TriggerDataSubChunk0x16(const size_t& trigger_count) {
        triggers.resize(trigger_count);
    }

    //implementation of pure virtual functions
    std::vector<TriggerDataEntry0x3>& get_container() override {
        return triggers;
    }
    
    const std::vector<TriggerDataEntry0x3>& get_container() const override {
        return triggers;
    }
};
#pragma pack(pop)

//individual good data structure (520 bytes per good)
struct GoodData0x4 {
    char16_t name[256];          // good name (512 bytes - wide characters)
    uint32_t encrypted_param1;  // Economic parameter 1 (XOR encrypted with 0x63637 based on code)
                                // likely price, quantity, or value???
    uint32_t encrypted_param2;  // Economic parameter 2 (XOR encrypted with 0x63637 based on code)
                                // likely effects, requirements, or modifiers???
    // Total: 520 bytes (0x208)
    
    //helper functions to decrypt the parameters
    uint32_t get_param1() const { return encrypted_param1 ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    uint32_t get_param2() const { return encrypted_param2 ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    
    void set_param1(uint32_t value) { encrypted_param1 = value ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    void set_param2(uint32_t value) { encrypted_param2 = value ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
};
static_assert(sizeof(GoodData0x4) == 520, "GoodData0x4 must be exactly 520 bytes");

#pragma pack(push, 1)
struct GoodsCountSubChunk0x17 : public ByteConvertible<GoodsCountSubChunk0x17> {
    uint32_t goods_count;    //number of active goods in the scenario
};
#pragma pack(pop)
static_assert(sizeof(GoodsCountSubChunk0x17) == 4, "GoodsCountSubChunk0x17 header size mismatch");

#pragma pack(push,1)
struct GoodsDataSubChunk0x18 : public VariableLengthArrayChunk<GoodsDataSubChunk0x18, GoodData0x4> {
    std::vector<GoodData0x4> goods;

    GoodsDataSubChunk0x18() = default;

    GoodsDataSubChunk0x18(const GoodsCountSubChunk0x17& header) {
        goods.resize(header.goods_count);
    }

    GoodsDataSubChunk0x18(const size_t& trigger_count) {
        goods.resize(trigger_count);
    }

    //implementation of pure virtual functions
    std::vector<GoodData0x4>& get_container() override {
        return goods;
    }
    
    const std::vector<GoodData0x4>& get_container() const override {
        return goods;
    }
};
#pragma pack(pop)

//good object vtable pointer for type identification
namespace GoodVTables {
    constexpr uintptr_t GOOD_VTABLE = 0xb447e8;  //Good::vftable address ...
}
