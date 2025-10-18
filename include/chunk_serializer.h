#pragma once
#include "data_structures.h"
#include <vector>
#include <cstdint>

/**
 * methods to serialize chunk trees and individual chunks back into the binary format, which is expected by RoN scenario files.
 */
class ChunkSerializer {
public:
    /**
     * Serialize a single chunk and all its children to binary format
     * @param chunk the chunk to serialize
     * @return binary data representing the chunk
     */
    static std::vector<uint8_t> SerializeChunk(const Chunk& chunk);
    
    /**
     * Serialize multiple root chunks to binary format
     * @param chunks vector of root chunks to serialize
     * @return binary data representing all chunks
     */
    static std::vector<uint8_t> SerializeChunks(const std::vector<Chunk>& chunks);
    
    /**
     * Recursively update chunk sizes after modifications. 
     * This must be called after modifying any chunk data to ensure the chunk_size field accurately reflects the total size
     * @param chunk the chunk to update (modified in-place)
     */
    static void UpdateChunkSizes(Chunk& chunk);
    
    /**
     * Calculate the total size of a chunk including header, children, and data
     * @param chunk the chunk to calculate size for
     * @return total size in bytes
     */
    static size_t CalculateChunkSize(const Chunk& chunk);

private:
    /**
     * helper to serialize chunk header
     * @param header the header to serialize
     * @return binary data of the header
     */
    static std::vector<uint8_t> SerializeHeader(const ChunkHeader& header);
};
