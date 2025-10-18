#include "chunk_validator.h"
#include "chunk_serializer.h"
#include <sstream>

bool ChunkValidator::ValidateChunkSize(const Chunk& chunk) {
    size_t actualSize = ChunkSerializer::CalculateChunkSize(chunk);
    return actualSize == chunk.header.chunk_size;
}

bool ChunkValidator::ValidateChildCount(const Chunk& chunk) {
    return chunk.children.size() == chunk.header.direct_ascending_chunks_count;
}

bool ChunkValidator::ValidateChunkTree(const Chunk& chunk) {
    //check the current chunk
    if(!ValidateChildCount(chunk)) {
        return false;
    }
    
    if(!ValidateChunkSize(chunk)) {
        return false;
    }
    
    //recursively check all children
    for(const auto& child : chunk.children) {
        if(!ValidateChunkTree(child)) {
            return false;
        }
    }
    
    return true;
}

void ChunkValidator::ValidateRecursive(const Chunk& chunk, ValidationResult& result, int depth) {
    std::ostringstream prefix;
    prefix << "Depth " << depth << ", Chunk 0x" << std::hex 
           << static_cast<uint16_t>(chunk.header.chunk_type_identifier) << ": ";
    
    //validate the child count
    if(chunk.children.size() != chunk.header.direct_ascending_chunks_count) {
        std::ostringstream error;
        error << prefix.str() << "Child count mismatch. Header says " 
              << chunk.header.direct_ascending_chunks_count 
              << " but found " << chunk.children.size();
        result.AddError(error.str());
    }
    
    //check for minimum size
    if(chunk.header.chunk_size < sizeof(ChunkHeader)) {
        std::ostringstream error;
        error << prefix.str() << "Chunk size (" << chunk.header.chunk_size 
              << ") is less than header size";
        result.AddError(error.str());
    }

    //validate the chunk size
    size_t actualSize = ChunkSerializer::CalculateChunkSize(chunk);
    if(actualSize != chunk.header.chunk_size) {
        std::ostringstream error;
        error << prefix.str() << "Size mismatch. Header says " 
              << chunk.header.chunk_size << " but calculated " << actualSize;
        result.AddError(error.str());
    }
    
    //warn if chunk has no data and no children
    if(chunk.data.empty() && chunk.children.empty()) {
        std::ostringstream warning;
        warning << prefix.str() << "Empty chunk (no data, no children)";
        result.AddWarning(warning.str());
    }
    
    //recursively validate children
    for(const auto& child : chunk.children) {
        ValidateRecursive(child, result, depth + 1);
    }
}

ChunkValidator::ValidationResult ChunkValidator::Validate(const Chunk& chunk) {
    ValidationResult result;
    ValidateRecursive(chunk, result, 0);
    return result;
}

ChunkValidator::ValidationResult ChunkValidator::ValidateScenario(const std::vector<Chunk>& chunks) {
    ValidationResult result;
    
    if(chunks.empty()) {
        result.AddWarning("Scenario has no chunks");
        return result;
    }
    
    for(size_t i = 0; i < chunks.size(); ++i) {
        std::ostringstream prefix;
        prefix << "Root chunk " << i << ": ";
        
        ValidationResult chunkResult = Validate(chunks[i]);
        
        //merge all errors
        for(const auto& error : chunkResult.errors) {
            result.AddError(prefix.str() + error);
        }
        
        //merge all warnings
        for(const auto& warning : chunkResult.warnings) {
            result.AddWarning(prefix.str() + warning);
        }
    }
    
    return result;
}
