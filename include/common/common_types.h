#pragma once
#include <cstdint>

//common color structure
#pragma pack(push, 1)
struct RGBColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};
#pragma pack(pop)
static_assert(sizeof(RGBColor) == 0x3, "RGBColor must be exactly 3 bytes");

//common coordinate structure
#pragma pack(push, 1)
struct Coordinate {
    int32_t X;
    int32_t Y;
};
#pragma pack(pop)

//tech tree type name structure (used across multiple chunks)
#pragma pack(push, 1)
struct TechTreeTypeName {
    char16_t name[256];
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
