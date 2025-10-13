#pragma once
#include "../chunk_types.h"
#include <vector>
#include <cstdint>

struct ChunkHeader {
    uint32_t chunk_size;
    ChunkType chunk_type_identifier;
    uint16_t direct_ascending_chunks_count;

    ChunkHeader() = default;
    ChunkHeader(uint32_t s, ChunkType t) : chunk_size(s), chunk_type_identifier(t) {}
};

struct Chunk {
    ChunkHeader header;
    std::vector<std::byte> data;
    std::vector<Chunk> children;
    size_t file_offset = 0;

    ~Chunk() = default;
    Chunk(const ChunkHeader& h) : header(h) {}
};
