#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>
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
enum class LibraryResearchCategory {
    Unknown = 0,
    AgeAdvances,
    Science,
    Commerce,
    Civic,
    Military
};

enum class LibraryResearchId {
    Unknown = 0,
    ClassicalAge,
    MedievalAge,
    GunpowderAge,
    EnlightenmentAge,
    IndustrialAge,
    ModernAge,
    InformationAge,
    WrittenWord,
    Mathematics,
    Chemistry,
    LawsOfNature,
    Electricity,
    Electronics,
    Computerization,
    Barter,
    Coinage,
    Trade,
    Mercantilism,
    Finance,
    AssemblyLine,
    Globalization,
    CityState,
    Empire,
    Feudalism,
    DivineRight,
    Constitution,
    GreatPower,
    InternationalLaw,
    TheArtOfWar,
    Mercenaries,
    StandingArmy,
    Conscription,
    LeveeEnMasse,
    NationInArms,
    SelectiveService
};

struct LibraryResearchInfo {
    LibraryResearchId id;
    LibraryResearchCategory category;
    const char16_t* name;
};

inline constexpr LibraryResearchInfo kKnownLibraryResearches[] = {
    {LibraryResearchId::ClassicalAge,      LibraryResearchCategory::AgeAdvances, u"Classical Age"},
    {LibraryResearchId::MedievalAge,       LibraryResearchCategory::AgeAdvances, u"Medieval Age"},
    {LibraryResearchId::GunpowderAge,      LibraryResearchCategory::AgeAdvances, u"Gunpowder Age"},
    {LibraryResearchId::EnlightenmentAge,  LibraryResearchCategory::AgeAdvances, u"Enlightenment Age"},
    {LibraryResearchId::IndustrialAge,     LibraryResearchCategory::AgeAdvances, u"Industrial Age"},
    {LibraryResearchId::ModernAge,         LibraryResearchCategory::AgeAdvances, u"Modern Age"},
    {LibraryResearchId::InformationAge,    LibraryResearchCategory::AgeAdvances, u"Information Age"},
    {LibraryResearchId::WrittenWord,       LibraryResearchCategory::Science,     u"Written Word"},
    {LibraryResearchId::Mathematics,       LibraryResearchCategory::Science,     u"Mathematics"},
    {LibraryResearchId::Chemistry,         LibraryResearchCategory::Science,     u"Chemistry"},
    {LibraryResearchId::LawsOfNature,      LibraryResearchCategory::Science,     u"Laws of Nature"},
    {LibraryResearchId::Electricity,       LibraryResearchCategory::Science,     u"Electricity"},
    {LibraryResearchId::Electronics,       LibraryResearchCategory::Science,     u"Electronics"},
    {LibraryResearchId::Computerization,   LibraryResearchCategory::Science,     u"Computerization"},
    {LibraryResearchId::Barter,            LibraryResearchCategory::Commerce,    u"Barter"},
    {LibraryResearchId::Coinage,           LibraryResearchCategory::Commerce,    u"Coinage"},
    {LibraryResearchId::Trade,             LibraryResearchCategory::Commerce,    u"Trade"},
    {LibraryResearchId::Mercantilism,      LibraryResearchCategory::Commerce,    u"Mercantilism"},
    {LibraryResearchId::Finance,           LibraryResearchCategory::Commerce,    u"Finance"},
    {LibraryResearchId::AssemblyLine,      LibraryResearchCategory::Commerce,    u"Assembly Line"},
    {LibraryResearchId::Globalization,     LibraryResearchCategory::Commerce,    u"Globalization"},
    {LibraryResearchId::CityState,         LibraryResearchCategory::Civic,       u"City State"},
    {LibraryResearchId::Empire,            LibraryResearchCategory::Civic,       u"Empire"},
    {LibraryResearchId::Feudalism,         LibraryResearchCategory::Civic,       u"Feudalism"},
    {LibraryResearchId::DivineRight,       LibraryResearchCategory::Civic,       u"Divine Right"},
    {LibraryResearchId::Constitution,      LibraryResearchCategory::Civic,       u"Constitution"},
    {LibraryResearchId::GreatPower,        LibraryResearchCategory::Civic,       u"Great Power"},
    {LibraryResearchId::InternationalLaw,  LibraryResearchCategory::Civic,       u"International Law"},
    {LibraryResearchId::TheArtOfWar,       LibraryResearchCategory::Military,    u"The Art of War"},
    {LibraryResearchId::Mercenaries,       LibraryResearchCategory::Military,    u"Mercenaries"},
    {LibraryResearchId::StandingArmy,      LibraryResearchCategory::Military,    u"Standing Army"},
    {LibraryResearchId::Conscription,      LibraryResearchCategory::Military,    u"Conscription"},
    {LibraryResearchId::LeveeEnMasse,      LibraryResearchCategory::Military,    u"Levee en Masse"},
    {LibraryResearchId::NationInArms,      LibraryResearchCategory::Military,    u"Nation-in-Arms"},
    {LibraryResearchId::SelectiveService,  LibraryResearchCategory::Military,    u"Selective Service"},
};

inline const char* libraryResearchCategoryName(LibraryResearchCategory category)
{
    switch (category) {
        case LibraryResearchCategory::AgeAdvances: return "Age Advances";
        case LibraryResearchCategory::Science:     return "Science";
        case LibraryResearchCategory::Commerce:    return "Commerce";
        case LibraryResearchCategory::Civic:       return "Civic";
        case LibraryResearchCategory::Military:    return "Military";
        default:                                   return "Unknown";
    }
}

inline const LibraryResearchInfo* findLibraryResearchInfo(std::u16string_view name)
{
    for (const auto& info : kKnownLibraryResearches) {
        if (name == std::u16string_view(info.name)) {
            return &info;
        }
    }
    return nullptr;
}

#pragma pack(push, 1)
struct TechTreeTypeName {
    char16_t name[256];

    std::u16string_view as_view() const {
        size_t length = 0;
        while (length < 256 && name[length] != u'\0') {
            ++length;
        }
        return std::u16string_view(name, length);
    }

    const LibraryResearchInfo* library_research_info() const {
        return findLibraryResearchInfo(as_view());
    }

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
