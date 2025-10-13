#include "scenario_parser.h"
#include "chunk_types.h"
#include <iomanip>

Chunk ScenarioParser::parseChunk(const std::vector<uint8_t>& fileData, size_t& offset) {
    if (offset + sizeof(ChunkHeader) > fileData.size()) {
        throw std::runtime_error("Insufficient data for chunk header at offset " + std::to_string(offset));
    }
    
    //read chunk header
    const ChunkHeader* headerPtr = reinterpret_cast<const ChunkHeader*>(fileData.data() + offset);
    Chunk chunk(*headerPtr);
    size_t chunkStart = offset;
    chunk.file_offset = chunkStart;
    offset += sizeof(ChunkHeader);
    
    //debug output
    std::cout << "Parsing chunk at offset " << chunkStart 
              << " - Type: 0x" << std::hex << static_cast<uint16_t>(chunk.header.chunk_type_identifier)
              << " Size: " << std::dec << chunk.header.chunk_size 
              << " Children: " << chunk.header.direct_ascending_chunks_count << std::endl;
    
    //validate chunk size
    if (chunk.header.chunk_size < sizeof(ChunkHeader)) {
        throw std::runtime_error("Invalid chunk size: " + std::to_string(chunk.header.chunk_size) + 
                               " at offset " + std::to_string(chunkStart));
    }
    if (chunkStart + chunk.header.chunk_size > fileData.size()) {
        throw std::runtime_error("Chunk extends beyond file: chunk ends at " + 
                               std::to_string(chunkStart + chunk.header.chunk_size) + 
                               " but file size is " + std::to_string(fileData.size()));
    }
    
    //reserve space for children
    chunk.children.reserve(chunk.header.direct_ascending_chunks_count);
    
    //parse child chunks first
    for (uint16_t i = 0; i < chunk.header.direct_ascending_chunks_count; ++i) {
        Chunk childChunk = parseChunk(fileData, offset);
        chunk.children.push_back(std::move(childChunk));
    }
    
    //calculate how much data is left for this chunk
    size_t consumedBytes = offset - chunkStart;
    if (consumedBytes > chunk.header.chunk_size) {
        throw std::runtime_error("Child chunks exceed parent chunk size: consumed " + 
                               std::to_string(consumedBytes) + " but chunk size is " + 
                               std::to_string(chunk.header.chunk_size));
    }
    size_t remainingDataSize = chunk.header.chunk_size - consumedBytes;
    
    //read remaining data
    if (remainingDataSize > 0) {
        if (offset + remainingDataSize > fileData.size()) {
            throw std::runtime_error("Insufficient data for chunk content: need " + 
                                   std::to_string(remainingDataSize) + " bytes at offset " + 
                                   std::to_string(offset) + " but only " + 
                                   std::to_string(fileData.size() - offset) + " bytes remaining");
        }
        chunk.data.resize(remainingDataSize);
        std::memcpy(chunk.data.data(), fileData.data() + offset, remainingDataSize);
        offset += remainingDataSize;
    }
    
    return chunk;
}

void ScenarioParser::printChunkTree(const Chunk& chunk, int depth) {
    std::string indent(depth * 4, ' ');
    std::string name_of_chunk = getChunkTypeName(chunk.header.chunk_type_identifier);
    
    std::cout << indent << "[0x" << std::hex << chunk.file_offset << "] "
              << "Chunk Type: " << name_of_chunk << " (0x" << std::hex << static_cast<uint16_t>(chunk.header.chunk_type_identifier)
              << ") Chunk_Size: " << std::dec << chunk.header.chunk_size 
              << " Bytes, Children: " << chunk.header.direct_ascending_chunks_count 
              << ", Data: " << chunk.data.size() << " bytes" << std::endl;
    
    for (const auto& child : chunk.children) {
        printChunkTree(child, depth + 1);
    }
}
