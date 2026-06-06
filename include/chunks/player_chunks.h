#pragma once
#include "../base/byte_convertible.h"
#include "../base/chunk_reflection.h"
#include "../common/common_types.h"
#include <cstdint>

#pragma pack(push, 1)
struct PlayerEntry {
    uint32_t player_index;      // player slot (0-7)
    uint32_t player_data_value; // data from game memory array
    uint32_t flags;             // combined status flags

    BEGIN_FIELD_DESCRIPTORS(PlayerEntry)
        DESCRIBE_FIELD(uint32_t, player_index, "Player slot (0-7)")
        DESCRIBE_FIELD(uint32_t, player_data_value, "Data from game memory")
        DESCRIBE_FIELD(uint32_t, flags, "Combined status flags")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
static_assert(sizeof(PlayerEntry) == 0xC, "PlayerEntry must be exactly 12 bytes");

#pragma pack(push, 1)
struct PlayerPropertiesChunk0x6B : public ByteConvertible<PlayerPropertiesChunk0x6B> {
    //looking at the code, these are the fields copied starting from local_15c:
    uint32_t player_index;           // +0x00: From player_data + 0xe3a398 - player owning this formation (0-7)
    uint32_t nation_index;           // +0x04: From player_data + 0xe3a39c - the nation the player is playing as
    /*
        AI Settings:
            Production AI:
                Toggled = 0
                Non-Toggled = 2
            Unit AI:
                Toggled = 0
                Non-Toggled = 1
            Combat AI:
                Toggled = 0
                Non-Toggled = 64
        
        Misc Settings:
            Auto-transport:
                Default = 0
                Never = 4
                Always = 396
    */
    uint32_t player_flags;          // +0x08: Computed flags (local_154)
    int32_t field_0x150;            // +0x0C: From player_data + 0xe3a3a4
    uint32_t field_0x14c;           // +0x10: From player_data + 0xe3a79c
    
    // player name buffer - local_148[100] in the code, but since wchar_t is 2 bytes, this is 200 bytes
    char16_t  player_name[100];      // +0x14: 200 bytes (confirmed from code)
    
    //remaining 16 bytes for other fields
    uint32_t control_field;         // +0xDC: 4 bytes (0 = computer, 4 = human)
    uint32_t remaining_field2;      // +0xE0: 4 bytes  
    uint8_t color_index;            // +0xE4: 1 bytes - color index (0 = red, 1 = blue, etc)
    uint8_t _pad_e5;                // +0xE5: uninitialized stack garbage in game writer - ignore
    uint8_t _pad_e6;                // +0xE6: uninitialized stack garbage in game writer - ignore
    uint8_t _pad_e7;                // +0xE7: uninitialized stack garbage in game writer - ignore
    /*
        0 = Easiest
        1 = Easy
        2 = Moderate
        3 = Tough
        4 = Tougher
        5 = Toughest
    */
    uint32_t difficulty;            // +0xE8: 4 bytes

    //Total amount of bytes: 20 + 200 + 16 = 236 bytes exactly

    BEGIN_FIELD_DESCRIPTORS(PlayerPropertiesChunk0x6B)
        DESCRIBE_FIELD(uint32_t, player_index, "Player slot (0-7)")
        DESCRIBE_FIELD(uint32_t, nation_index, "Nation the player is playing")
        DESCRIBE_FIELD(uint32_t, player_flags, "AI/misc settings flags")
        DESCRIBE_FIELD(int32_t, field_0x150, "Field from offset 0x150")
        DESCRIBE_FIELD(uint32_t, field_0x14c, "Field from offset 0x14c")
        DESCRIBE_FIELD(char16_t[100], player_name, "Player name (UTF-16)")
        DESCRIBE_FIELD(uint32_t, control_field, "0=computer, 4=human")
        DESCRIBE_FIELD(uint32_t, remaining_field2, "Additional field")
        DESCRIBE_FIELD(uint8_t, color_index, "Color (0=red, 1=blue, etc)")
        DESCRIBE_FIELD(uint8_t, _pad_e5, "Color component 1")
        DESCRIBE_FIELD(uint8_t, _pad_e6, "Color component 2")
        DESCRIBE_FIELD(uint8_t, _pad_e7, "Color component 3")
        DESCRIBE_FIELD(uint32_t, difficulty, "0-5 (Easiest to Toughest)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
static_assert(sizeof(PlayerPropertiesChunk0x6B) == 236, "PlayerPropertiesChunk0x6B must be exactly 236 bytes");

//player flag constants (used in player_flags field)
namespace PlayerFlags {
    const uint32_t FLAG_0x001 = 0x001;  // set when (player_data + 0xe3a394) & 2
    const uint32_t FLAG_0x002 = 0x002;  // set when (player_data + 0xe3a394) & 4
    const uint32_t FLAG_0x004 = 0x004;  // set when (player_data + 0xe3a394) & 0x20
    const uint32_t FLAG_0x008 = 0x008;  // set when player_flags & 0x100
    const uint32_t FLAG_0x010 = 0x010;  // set when player_flags & 0x100000
    const uint32_t FLAG_0x020 = 0x020;  // set when player_flags & 0x200000
    const uint32_t FLAG_0x040 = 0x040;  // set when (player_data + 0xe3a394) & 8
    const uint32_t FLAG_0x080 = 0x080;  // set when player_flags & 0x200
    const uint32_t FLAG_0x100 = 0x100;  // set when player_flags & 0x400
    const uint32_t FLAG_0x200 = 0x200;  // set when (player_data + 0xe3a394) & 1
    const uint32_t FLAG_0x400 = 0x400;  // set when player_flags & 8
}

#pragma pack(push, 1)
struct PlayerCountChunk0x2a : public ByteConvertible<PlayerCountChunk0x2a> {
    uint32_t player_count; //number of players

    BEGIN_FIELD_DESCRIPTORS(PlayerCountChunk0x2a)
        DESCRIBE_FIELD(uint32_t, player_count, "Number of active players")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
static_assert(sizeof(PlayerCountChunk0x2a) == 0x4, "PlayerCountChunk0x2a must be exactly 4 bytes");

#pragma pack(push, 1)
struct PlayerDetailsChunk0x2b : public VariableLengthArrayChunk<PlayerDetailsChunk0x2b, PlayerEntry> {
    std::vector<PlayerEntry> player_entries;

    PlayerDetailsChunk0x2b() = default;

    PlayerDetailsChunk0x2b(const PlayerCountChunk0x2a& header) {
        player_entries.resize(header.player_count);
    }

    PlayerDetailsChunk0x2b(const size_t& count) {
        player_entries.resize(count);
    }

    //implementation of pure virtual functions
    std::vector<PlayerEntry>& get_container() override {
        return player_entries;
    }
    
    const std::vector<PlayerEntry>& get_container() const override {
        return player_entries;
    }
};
#pragma pack(pop)
