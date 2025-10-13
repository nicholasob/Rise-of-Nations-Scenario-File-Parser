#include "chunk_parsers.h"

bool ChunkParsers::parseMapStructure(const Chunk& chunk, uint32_t& total_tiles, uint32_t& width, uint32_t& height) {
    if (chunk.data.size() >= 12) {
        const uint32_t* data = reinterpret_cast<const uint32_t*>(chunk.data.data());
        total_tiles = data[0];
        width = data[1];
        height = data[2];
        return true;
    }
    return false;
}

std::string ChunkParsers::ConvertChar16ToString(const char16_t* str, size_t maxSize) {
    std::string result;
    for (size_t i = 0; i < maxSize && str[i] != u'\0'; ++i) {
        if (str[i] <= 0x7F) {
            result += static_cast<char>(str[i]);
        } else {
            result += '?'; //fallback for non-ASCII
        }
    }
    return result;
}

std::vector<std::pair<uint32_t, uint32_t>> ChunkParsers::DEPRECATED_parseResourceEntries(const Chunk& chunk) {
    std::vector<std::pair<uint32_t, uint32_t>> resources;
    if (chunk.data.size() >= 8) {
        size_t count = chunk.data.size() / 8;
        const uint32_t* data = reinterpret_cast<const uint32_t*>(chunk.data.data());
        
        for (size_t i = 0; i < count; ++i) {
            uint32_t index = data[i * 2];
            uint32_t amount = data[i * 2 + 1]; //seems to not be encrypted? whast with the XOR encryption
            resources.emplace_back(index, amount);
        }
    }
    return resources;
}

std::string ChunkParsers::decodePlayerFlags(uint32_t flags)

    {
        std::string result = "";
        if(flags & PlayerFlags::FLAG_0x001) result += "FLAG_001 ";
        if(flags & PlayerFlags::FLAG_0x002) result += "FLAG_002 ";
        if(flags & PlayerFlags::FLAG_0x004) result += "FLAG_004 ";
        if(flags & PlayerFlags::FLAG_0x008) result += "FLAG_008 ";
        if(flags & PlayerFlags::FLAG_0x010) result += "FLAG_010 ";
        if(flags & PlayerFlags::FLAG_0x020) result += "FLAG_020 ";
        if(flags & PlayerFlags::FLAG_0x040) result += "FLAG_040 ";
        if(flags & PlayerFlags::FLAG_0x080) result += "FLAG_080 ";
        if(flags & PlayerFlags::FLAG_0x100) result += "FLAG_100 ";
        if(flags & PlayerFlags::FLAG_0x200) result += "FLAG_200 ";
        if(flags & PlayerFlags::FLAG_0x400) result += "FLAG_400 ";
        return result;
    };

