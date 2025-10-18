#include "chunk_serializer.h"
#include <cstring>
#include <stdexcept>


size_t ChunkSerializer::CalculateChunkSize(const Chunk& chunk) {
    size_t total = sizeof(ChunkHeader);
    
    //add size of all children recursively
    for(const auto& child : chunk.children) {
        total += CalculateChunkSize(child);
    }
    
    //add the current chunk's data portion size
    total += chunk.data.size();
    
    return total;
}

void ChunkSerializer::UpdateChunkSizes(Chunk& chunk) {
    //1. recursively update all children of the chunk
    for(auto& child : chunk.children) {
        UpdateChunkSizes(child);
    }
    
    //2. calculate the total size for the current chunk
    size_t totalSize = sizeof(ChunkHeader);
    
    //3. add the sizes of the children
    for(const auto& child : chunk.children) {
        totalSize += child.header.chunk_size;
    }
    
    //4. add the current chunk's data portion size
    totalSize += chunk.data.size();
    
    //5. update the header final size
    chunk.header.chunk_size = static_cast<uint32_t>(totalSize);
}


std::vector<uint8_t> ChunkSerializer::SerializeHeader(const ChunkHeader& header) {
    std::vector<uint8_t> result(sizeof(ChunkHeader));
    std::memcpy(result.data(), &header, sizeof(ChunkHeader));
    return result;
}

std::vector<uint8_t> ChunkSerializer::SerializeChunk(const Chunk& chunk) {
    std::vector<uint8_t> result;
    
    //reserve space to minimize the amount of reallocations
    result.reserve(chunk.header.chunk_size);
    
    //1. serialize the header
    auto headerData = SerializeHeader(chunk.header);
    result.insert(result.end(), headerData.begin(), headerData.end());
    
    //2. serialize all children (in order)
    for(const auto& child : chunk.children) {
        auto childData = SerializeChunk(child);
        result.insert(result.end(), childData.begin(), childData.end());
    }
    
    //3. serialize the chunk's data portion
    if(!chunk.data.empty()) {
        //casting std::byte to uint8_t
        for(const auto& byte : chunk.data) {
            result.push_back(static_cast<uint8_t>(byte));
        }
    }
    
    //verify that the size matches what we expect
    if(result.size() != chunk.header.chunk_size) {
        throw std::runtime_error(
            "Serialization error: expected " + std::to_string(chunk.header.chunk_size) + " bytes but got " + std::to_string(result.size()) + " bytes!"
        );
    }
    
    return result;
}

std::vector<uint8_t> ChunkSerializer::SerializeChunks(const std::vector<Chunk>& chunks) {
    std::vector<uint8_t> result;
    
    //calculate the total size for reservation
    size_t totalSize = 0;
    for(const auto& chunk : chunks) {
        totalSize += chunk.header.chunk_size;
    }
    result.reserve(totalSize);
    
    //serialize each root chunks
    for(const auto& chunk : chunks) {
        auto chunkData = SerializeChunk(chunk);
        result.insert(result.end(), chunkData.begin(), chunkData.end());
    }
    
    return result;
}
