#pragma once
#include "data_structures.h"
#include "chunk_types.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstddef>

class ScenarioParser {
public:
    //parse a single chunk from file data starting at a specific offset
    static Chunk ParseChunk(const std::vector<uint8_t>& fileData, size_t& offset);
    
    //print the chunk tree structure for debugging
    static void PrintChunkTree(const Chunk& chunk, int depth = 0);
    
    /**
     * Save a scenario to a file
     * @param chunks the root chunks to save
     * @param outputPath output file path
     * @param compress whether to compress with gzip (default true)
     * @return true if successful
     */
    static bool SaveScenario(const std::vector<Chunk>& chunks, const std::string& outputPath, bool compress = true);
    
    /**
     * Find the first chunk of a specific type in a chunk tree
     * @param chunk root chunk to search in
     * @param type chunk type to find
     * @return pointer to chunk or nullptr if not found
     */
    static Chunk* FindChunkByType(Chunk& chunk, ChunkType type);
    
    /**
     * Find all chunks of a specific type in a chunk tree
     * @param chunk root chunk to search in
     * @param type chunk type to find
     * @return vector of pointers to matching chunks
     */
    static std::vector<Chunk*> FindAllChunksByType(Chunk& chunk, ChunkType type);
    
    /**
     * Update chunk data with new binary content
     * @param chunk chunk to update
     * @param newData new data to set
     * @return true if successful
     */
    static bool UpdateChunkData(Chunk& chunk, const std::vector<std::byte>& newData);

private:
    static void FindAllChunksRecursive(Chunk& chunk, ChunkType type, std::vector<Chunk*>& results);
};
