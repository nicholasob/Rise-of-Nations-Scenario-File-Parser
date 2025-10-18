#include "scenario_parser.h"
#include "chunk_types.h"
#include "chunk_serializer.h"
#include "compression.h"
#include <iomanip>
#include <fstream>

Chunk ScenarioParser::ParseChunk(const std::vector<uint8_t>& fileData, size_t& offset) {
    if(offset + sizeof(ChunkHeader) > fileData.size()) {
        throw std::runtime_error("Insufficient data for chunk header at offset " + std::to_string(offset));
    }
    
    //read the chunk header
    const ChunkHeader* headerPtr = reinterpret_cast<const ChunkHeader*>(fileData.data() + offset);
    Chunk chunk(*headerPtr);
    size_t chunkStart = offset;
    chunk.file_offset = chunkStart;
    offset += sizeof(ChunkHeader);
    
    std::cout << "Parsing chunk at offset " << chunkStart 
              << " - Type: 0x" << std::hex << static_cast<uint16_t>(chunk.header.chunk_type_identifier)
              << " Size: " << std::dec << chunk.header.chunk_size 
              << " Children: " << chunk.header.direct_ascending_chunks_count << std::endl;
    
    //validate the chunk size
    if(chunk.header.chunk_size < sizeof(ChunkHeader)) {
        throw std::runtime_error("Invalid chunk size: " + std::to_string(chunk.header.chunk_size) + 
                               " at offset " + std::to_string(chunkStart));
    }
    if(chunkStart + chunk.header.chunk_size > fileData.size()) {
        throw std::runtime_error("Chunk extends beyond file: chunk ends at " + 
                               std::to_string(chunkStart + chunk.header.chunk_size) + 
                               " but file size is " + std::to_string(fileData.size()));
    }
    
    //reserve space for children
    chunk.children.reserve(chunk.header.direct_ascending_chunks_count);
    
    //parse the child chunks first
    for(uint16_t i = 0; i < chunk.header.direct_ascending_chunks_count; ++i) {
        Chunk childChunk = ParseChunk(fileData, offset);
        chunk.children.push_back(std::move(childChunk));
    }
    
    //calculate how much data is left for this chunk
    size_t consumedBytes = offset - chunkStart;
    if(consumedBytes > chunk.header.chunk_size) {
        throw std::runtime_error("Child chunks exceed parent chunk size: consumed " + 
                               std::to_string(consumedBytes) + " but chunk size is " + 
                               std::to_string(chunk.header.chunk_size));
    }
    size_t remainingDataSize = chunk.header.chunk_size - consumedBytes;
    
    //read remaining data
    if(remainingDataSize > 0) {
        if(offset + remainingDataSize > fileData.size()) {
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

void ScenarioParser::PrintChunkTree(const Chunk& chunk, int depth) {
    std::string indent(depth * 4, ' ');
    std::string name_of_chunk = getChunkTypeName(chunk.header.chunk_type_identifier);
    
    std::cout << indent << "[0x" << std::hex << chunk.file_offset << "] "
              << "Chunk Type: " << name_of_chunk << " (0x" << std::hex << static_cast<uint16_t>(chunk.header.chunk_type_identifier)
              << ") Chunk_Size: " << std::dec << chunk.header.chunk_size 
              << " Bytes, Children: " << chunk.header.direct_ascending_chunks_count 
              << ", Data: " << chunk.data.size() << " bytes" << std::endl;
    
    for(const auto& child : chunk.children) {
        PrintChunkTree(child, depth + 1);
    }
}

bool ScenarioParser::SaveScenario(const std::vector<Chunk>& chunks, const std::string& outputPath, bool compress) {
    try {
        //serialize the chunks
        std::vector<uint8_t> serialized = ChunkSerializer::SerializeChunks(chunks);
        
        //compress if requested
        std::vector<uint8_t> outputData;
        if(compress && !GzipHelper::compressGzip(serialized, outputData)) {
            std::cerr << "Failed to compress data" << std::endl;
            return false;
        }
        else {
            outputData = std::move(serialized);
        }
        
        //write to file
        std::ofstream outFile(outputPath, std::ios::binary);
        if(!outFile) {
            std::cerr << "Failed to open output file: " << outputPath << std::endl;
            return false;
        }
        
        outFile.write(reinterpret_cast<const char*>(outputData.data()), static_cast<std::streamsize>(outputData.size()));
        outFile.close();
        
        return outFile.good();
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving scenario: " << e.what() << std::endl;
        return false;
    }
}

Chunk* ScenarioParser::FindChunkByType(Chunk& chunk, ChunkType type) {
    if(chunk.header.chunk_type_identifier == type) {
        return &chunk;
    }
    
    for(auto& child : chunk.children) {
        Chunk* result = FindChunkByType(child, type);
        if(result != nullptr) {
            return result;
        }
    }
    
    return nullptr;
}

void ScenarioParser::FindAllChunksRecursive(Chunk& chunk, ChunkType type, std::vector<Chunk*>& results) {
    if(chunk.header.chunk_type_identifier == type) {
        results.push_back(&chunk);
    }
    
    for(auto& child : chunk.children) {
        FindAllChunksRecursive(child, type, results);
    }
}

std::vector<Chunk*> ScenarioParser::FindAllChunksByType(Chunk& chunk, ChunkType type) {
    std::vector<Chunk*> results;
    FindAllChunksRecursive(chunk, type, results);
    return results;
}

bool ScenarioParser::UpdateChunkData(Chunk& chunk, const std::vector<std::byte>& newData) {
    chunk.data = newData;
    
    //update the chunk size to match the new data
    ChunkSerializer::UpdateChunkSizes(chunk);
    
    return true;
}
