#pragma once
#include <cstdint>
#include "../base/chunk_reflection.h"

//common color structure
#pragma pack(push, 1)
struct RGBColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    BEGIN_FIELD_DESCRIPTORS(RGBColor)
        DESCRIBE_FIELD(uint8_t, r, "Red component (0-255)")
        DESCRIBE_FIELD(uint8_t, g, "Green component (0-255)")
        DESCRIBE_FIELD(uint8_t, b, "Blue component (0-255)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
static_assert(sizeof(RGBColor) == 0x3, "RGBColor must be exactly 3 bytes");

//common coordinate structure
#pragma pack(push, 1)
struct Coordinate {
    int32_t X;
    int32_t Y;

    Coordinate(int32_t x, int32_t y){
        this->X = x;
        this->Y = y;
    }
};
#pragma pack(pop)

// Tech tree name structure (used across multiple chunks).
// For TECH_TREE_UNIT_TYPE_NAMES this currently appears to store nation
// Library research names, observed in this order:
// Age Advances: Classical Age, Medieval Age, Gunpowder Age,
// Enlightenment Age, Industrial Age, Modern Age, Information Age
// Science: Written Word, Mathematics, Chemistry, Laws of Nature,
// Electricity, Electronics, Computerization
// Commerce: Barter, Coinage, Trade, Mercantilism, Finance,
// Assembly Line, Globalization
// Civic: City State, Empire, Feudalism, Divine Right, Constitution,
// Great Power, International Law
// Military: The Art of War, Mercenaries, Standing Army, Conscription,
// Levee en Masse, Nation-in-Arms, Selective Service
#pragma pack(push, 1)
struct TechTreeTypeName {
    char16_t name[256];

    BEGIN_FIELD_DESCRIPTORS(TechTreeTypeName)
        DESCRIBE_FIELD(char16_t[256], name, "Tech tree name / Library research name (UTF-16)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
static_assert(sizeof(TechTreeTypeName) == 0x200, "TechTreeTypeName must be exactly 512 bytes");

//wide string utility class
struct WideString {
private:
    static constexpr size_t MAX_LENGTH = 256;
    static constexpr size_t BUFFER_SIZE = 512; //256*2 bytes
public:
    static std::wstring ConvertToWideString(char16_t* chars) {
        // Convert to wstring
        std::wstring wname;
        for (int i = 0; i < 257 && chars[i] != u'\0'; ++i) {
            wname += static_cast<wchar_t>(chars[i]);
        }
        return wname;
    }
};
