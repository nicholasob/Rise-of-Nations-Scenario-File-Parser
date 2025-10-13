#pragma once
#include "data_structures.h"
#include "include/common/encryption.h"
#include <iostream>
#include <utility>
#include <algorithm>

//specialized parsers for more complex chunks
class ChunkParsers {
public:
    //parse map structure from chunk data
    static bool parseMapStructure(const Chunk& chunk, uint32_t& total_tiles, uint32_t& width, uint32_t& height);
    
    static std::string ConvertChar16ToString(const char16_t* str, size_t maxSize);

    //parse resource entries
    static std::vector<std::pair<uint32_t, uint32_t>> DEPRECATED_parseResourceEntries(const Chunk& chunk);
    
    static std::string decodePlayerFlags(uint32_t flags);
};


class PlayerPropertiesChunkParsers {
public:
    //public enums/structs
    enum class AutoTransport : uint8_t { Default, Never, Always, Unknown };

    struct DecodedPlayerFlags {
        bool unit_ai_toggled;        //true = toggled/on
        bool production_ai_toggled;  //true = toggled/on
        bool combat_ai_toggled;      //true = toggled/on
        AutoTransport auto_transport;
    };

    //public API (the "listed functions")
    static DecodedPlayerFlags DecodePlayerFlags(uint32_t flags) {
        const bool unit_on       = (flags & Flags::UnitAiNonToggled) == 0;
        const bool production_on = (flags & Flags::ProductionAiNonToggled) == 0;
        const bool combat_on     = (flags & Flags::CombatAiNonToggled) == 0;

        const uint32_t at = flags & Flags::AutoTransportMask;
        AutoTransport auto_transport =
            (at == Flags::AutoTransportDefault) ? AutoTransport::Default :
            (at == Flags::AutoTransportNever)   ? AutoTransport::Never   :
            (at == Flags::AutoTransportAlways)  ? AutoTransport::Always  :
                                                  AutoTransport::Unknown;

        return { unit_on, production_on, combat_on, auto_transport };
    }

    static uint32_t EncodePlayerFlags(const DecodedPlayerFlags& d) {
        uint32_t flags = 0;

        if (!d.unit_ai_toggled)       flags |= Flags::UnitAiNonToggled;
        if (!d.production_ai_toggled) flags |= Flags::ProductionAiNonToggled;
        if (!d.combat_ai_toggled)     flags |= Flags::CombatAiNonToggled;

        // write auto-transport (clear field then set)
        flags &= ~Flags::AutoTransportMask;
        switch (d.auto_transport) {
            case AutoTransport::Default: /*nothing*/ break;
            case AutoTransport::Never:   flags |= Flags::AutoTransportNever;  break;
            case AutoTransport::Always:  flags |= Flags::AutoTransportAlways; break;
            default: throw std::runtime_error("Unknown AutoTransport cannot be encoded");
        }
        return flags;
    }

    static const char* ToString(AutoTransport at) {
        switch (at) {
            case AutoTransport::Default: return "Default";
            case AutoTransport::Never:   return "Never";
            case AutoTransport::Always:  return "Always";
            default:                     return "Unknown";
        }
    }

private:
    //centralizes bit definitions to reduce scatter and mistakes.
    struct Flags {
        //single-bit toggles: bit set = non-toggled(off), bit clear = toggled(on)
        static inline constexpr uint32_t UnitAiNonToggled       = 0x00000001u;
        static inline constexpr uint32_t ProductionAiNonToggled = 0x00000002u;
        static inline constexpr uint32_t CombatAiNonToggled     = 0x00000040u;

        //multi-bit field for auto-transport
        static inline constexpr uint32_t AutoTransportMask      = 0x0000018Cu; //uses bits 2, 3, 7, 8
        static inline constexpr uint32_t AutoTransportDefault   = 0x00000000u;
        static inline constexpr uint32_t AutoTransportNever     = 0x00000004u;
        static inline constexpr uint32_t AutoTransportAlways    = 0x0000018Cu;
    };
};