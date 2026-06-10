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

    /*
        11 = In-game Diplomacy (OFF)
        12 = In-game Diplomacy (ON)
    */
    uint32_t player_setting1;     // local_190 = PTR_DAT_00c061ec[0x24]
    /*
        0 = Standard
        1 = Custom
        2 = No Rush
        3 = Sudden Death
        4 = Deathmatch
        5 = Nomad
        6 = Peaceful Tech Race
        7 = Assassin!
        8 = Barbarians at the Gates!
        9 = Fast Game
        10 = Scenario
        11 = Info Age Deathmatch
    */
    uint32_t game_rules;          // local_18c = PTR_DAT_00c061ec[0x2a]
    uint32_t player_setting3;     // local_188 = PTR_DAT_00c061ec[0x25]
    uint32_t player_setting4;     // local_184 = PTR_DAT_00c061ec[0x26]

    uint32_t global_setting;      // local_180 = *(PTR_DAT_00c06188 + 0x7c)

    /*
        &= 0001 (Load Scenario Script)
        &= 0010 (Allow Rule Overrides)
        &= 0100 (Disable Unique Units)
        &= 1000 (Disable Nation Powers)
    */
    uint32_t game_flags;          // local_17c — various game mode flags
                                  // plus:
                                  // + |= 4 if (PTR_DAT_00c061ec[0x20] & 8)
                                  // + |= 8 if (PTR_DAT_00c061ec[0x20] & 4)
                                  // + |= 2 if (some global != 0)

    uint32_t ai_difficulty;       // local_178 = PTR_DAT_00c061ec[0x29]
    /*
        0 = Ancient Age
        1 = Classical Age
        2 = Medieval Age
        3 = Gunpowder Age
        4 = Enlightenment Age
        5 = Industrial Age
        6 = Modern Age
        7 = Information Age
        8 = All Technologies
    */
    uint32_t start_age;           // local_174 = PTR_DAT_00c061ec[0x34]  -  LOW: 0, HIGH: 7
    /*
        0 = Ancient Age
        1 = Classical Age
        2 = Medieval Age
        3 = Gunpowder Age
        4 = Enlightenment Age
        5 = Industrial Age
        6 = Modern Age
        7 = Information Age
        8 = All Technologies
    */
    uint32_t end_age;             // local_170 = PTR_DAT_00c061ec[0x36]  -  LOW: 0, HIGH: 7
    /*
        0 = No Ally LOS
        1 = Normal
        2 = Explored
        3 = All visible
    */
    uint32_t reveal_map;            // local_16c = PTR_DAT_00c061ec[0x30]
    /*
        0 = Conquest
        1 = Capital
        2 = Sudden Death Capital
        3 = Sudden Death
    */
    uint32_t elimination;            // local_168 = PTR_DAT_00c061ec[0x37]
    /*
        0 = Standard
        1 = Sudden Death
        2 = Conquest
        3 = Score
        4 = Time Limit
        5 = Musical Chairs
        6 = Wonder
        7 = Territory
        8 = Economic
        9 = Tech Race
        10 = Scenario Victory
    */
    uint32_t victory;            // local_164 = PTR_DAT_00c061ec[0x38]
    /*
        0 = 1000
        1 = 2000
        2 = 3000
        3 = 4000
        4 = 5000
        5 = 6000
        6 = 7000
        7 = 8000
        8 = 9000
        9 = 10000
        10 = 15000
        11 = 20000
    */
    uint32_t score;            // local_160 = PTR_DAT_00c061ec[0x3a]
    /*
        0 = 15 Minutes
        1 = 30 Minutes
        2 = 45 Minutes
        3 = 60 Minutes
        4 = 90 Minutes
        5 = 2 Hours
        6 = 3 Hours
        7 = 4 Hours
        8 = Custom Scenario
    */
    uint32_t time_limit;            // local_15c = PTR_DAT_00c061ec[0x3c]
    /*
        0 = 1 Wonder Point
        1 = 2 Wonder Points
        2 = 3 Wonder Points
        3 = 4 Wonder Points
        4 = 6 Wonder Points
        5 = 8 Wonder Points
        6 = 10 Wonder Points
        7 = 12 Wonder Points
        8 = 14 Wonder Points
        9 = 16 Wonder Points
        10 = 20 Wonder Points
        11 = 24 Wonder Points
        12 = No Wonder Victory
    */
    uint32_t wonder_victory;            // local_158 = PTR_DAT_00c061ec[0x39]
    /*
        0 = 30% of World Territory
        1 = 35% of World Territory
        2 = 40% of World Territory
        3 = 45% of World Territory
        4 = 50% of World Territory
        5 = 55% of World Territory
        6 = 60% of World Territory
        7 = 65% of World Territory
        8 = 70% of World Territory
        9 = 75% of World Territory
        10 = 80% of World Territory
        11 = 90% of World Territory
        12 = No Territory Victory
    */
    uint32_t territory_goal;            // local_154 = PTR_DAT_00c061ec[0x3b]
    /*
        0 = +100 Average Income
        1 = +200 Average Income
        2 = +300 Average Income
        3 = +400 Average Income
        4 = +500 Average Income
        5 = +600 Average Income
        6 = +700 Average Income
        7 = +800 Average Income
        8 = +900 Average Income
    */
    uint32_t income_goal;            // local_150 = PTR_DAT_00c061ec[0x3e]
    uint32_t player_setting5;     // local_14c = PTR_DAT_00c061ec[0x32]

    //global blob 1 (DAT_00cc22e0 - 00cc22ec)
    uint32_t camera_start_position_X_nation_1;           // local_148
    uint32_t camera_start_position_X_nation_2;           // uStack_144
    uint32_t camera_start_position_X_nation_3;           // uStack_140
    uint32_t camera_start_position_X_nation_4;           // uStack_13c

    //global blob 2 (DAT_00cc22f0 - 00cc22fc)
    uint32_t camera_start_position_X_nation_5;           // local_138
    uint32_t camera_start_position_X_nation_6;           // uStack_134
    uint32_t camera_start_position_X_nation_7;           // uStack_130
    uint32_t camera_start_position_X_nation_8;           // uStack_12c

    //global blob 3 (DAT_00cc0300 - 00cc030c)
    uint32_t camera_start_position_Y_nation_1;           // local_128
    uint32_t camera_start_position_Y_nation_2;           // uStack_124
    uint32_t camera_start_position_Y_nation_3;           // uStack_120
    uint32_t camera_start_position_Y_nation_4;           // uStack_11c

    //global blob 4 (DAT_00cc0310 - 00cc031c)
    uint32_t camera_start_position_Y_nation_5;           // local_118
    uint32_t camera_start_position_Y_nation_6;           // uStack_114
    uint32_t camera_start_position_Y_nation_7;           // uStack_110
    uint32_t camera_start_position_Y_nation_8;           // uStack_10c

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
    /*
        0 = 50
        1 = 75
        2 = 100
        3 = 125
        4 = 150
        5 = 200
    */
    uint32_t Population;          // local_e4 = PTR_DAT_00c061ec[0x31]
};
#pragma pack(pop)
static_assert(sizeof(ScenarioHeader0x6C) == 384, "ScenarioHeader must be exactly 384 bytes");

//individual trigger data entry (520 bytes each)
struct TriggerDataEntry0x3 {
    char16_t trigger_name[256];   // 512 bytes - trigger name/description
    uint32_t x_coordinate;        // 4 bytes - likely X position for trigger-linked placements such as Goody Box
    uint32_t y_coordinate;        // 4 bytes - likely Y position for trigger-linked placements such as Goody Box

    // Previous working assumption kept for reference:
    // these fields were named encrypted_param1 / encrypted_param2 and treated
    // as XOR-encrypted condition/action payloads with key 0x63637.
    uint32_t get_param1() const { return x_coordinate ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    uint32_t get_param2() const { return y_coordinate ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    void set_param1(uint32_t value) { x_coordinate = value ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
    void set_param2(uint32_t value) { y_coordinate = value ^ EncryptionHelper::PLAYER_BUILDING_DATA_XOR_KEY; }
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

// Individual map resource placement (520 bytes each).
// Observed names from the editor include:
// Relics, Wine, Papyrus, Dye, Silk, Spice, Salt, Horses, Rubber,
// Sulphur, Amber, Silver, Tobacco, Marble, Furs, Cotton, Bison,
// Copper, Peacocks, Citrus, Sugar, Diamonds, Gems, Aluminum,
// Obsidian, Wool, Coal, Uranium, Titanium, Fish, and Whales.
struct MapResourceData0x4 {
    char16_t name[256];   // map resource type name (512 bytes, UTF-16LE)
    uint32_t position_x;  // map resource X coordinate
    uint32_t position_y;  // map resource Y coordinate
    // Total: 520 bytes (0x208)
};
static_assert(sizeof(MapResourceData0x4) == 520, "MapResourceData0x4 must be exactly 520 bytes");

#pragma pack(push, 1)
struct MapResourceCountSubChunk0x17 : public ByteConvertible<MapResourceCountSubChunk0x17> {
    uint32_t map_resource_count;    // number of active map resource placements in the scenario
};
#pragma pack(pop)
static_assert(sizeof(MapResourceCountSubChunk0x17) == 4, "MapResourceCountSubChunk0x17 header size mismatch");

#pragma pack(push,1)
struct MapResourceDataSubChunk0x18 : public VariableLengthArrayChunk<MapResourceDataSubChunk0x18, MapResourceData0x4> {
    std::vector<MapResourceData0x4> map_resources;

    MapResourceDataSubChunk0x18() = default;

    MapResourceDataSubChunk0x18(const MapResourceCountSubChunk0x17& header) {
        map_resources.resize(header.map_resource_count);
    }

    MapResourceDataSubChunk0x18(const size_t& trigger_count) {
        map_resources.resize(trigger_count);
    }

    //implementation of pure virtual functions
    std::vector<MapResourceData0x4>& get_container() override {
        return map_resources;
    }
    
    const std::vector<MapResourceData0x4>& get_container() const override {
        return map_resources;
    }
};
#pragma pack(pop)

//good object vtable pointer for type identification
namespace MapResourceVTables {
    constexpr uintptr_t GOOD_VTABLE = 0xb447e8;  //Good::vftable address ...
}
