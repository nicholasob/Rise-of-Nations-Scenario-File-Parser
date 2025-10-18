#pragma once
#include "data_structures.h"
#include <vector>
#include <string>

/**
 * ensures that chunks maintain proper structure and size constraints after modifications.
 */
class ChunkValidator {
public:
    struct ValidationResult {
        bool valid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        
        ValidationResult() : valid(true) {}
        
        void AddError(const std::string& error) {
            errors.push_back(error);
            valid = false;
        }
        
        void AddWarning(const std::string& warning) {
            warnings.push_back(warning);
        }
    };
    
    /**
     * Validate a complete chunk tree
     * @param chunk the root chunk to validate
     * @return ValidationResult with any errors or warnings
     */
    static ValidationResult Validate(const Chunk& chunk);
    
    /**
     * Validate all root chunks in a scenario
     * @param chunks vector of root chunks
     * @return ValidationResult with any errors or warnings
     */
    static ValidationResult ValidateScenario(const std::vector<Chunk>& chunks);
    
    /**
     * Check if chunk size field matches actual calculated size
     * @param chunk the chunk to check
     * @return true if sizes match
     */
    static bool ValidateChunkSize(const Chunk& chunk);
    
    /**
     * Check if chunk tree structure is valid
     * @param chunk the chunk to check
     * @return true if structure is valid
     */
    static bool ValidateChunkTree(const Chunk& chunk);
    
    /**
     * Validate that the number of children matches the header count
     * @param chunk the chunk to check
     * @return true if child count is correct
     */
    static bool ValidateChildCount(const Chunk& chunk);

private:
    /**
     * Internal recursive validation helper
     * @param chunk the chunk to validate
     * @param result the result object to populate
     * @param depth Current depth in the tree (for error messages)
     */
    static void ValidateRecursive(const Chunk& chunk, ValidationResult& result, int depth = 0);
    
    /**
     * Calculate actual size of chunk including all components
     * @param chunk the chunk to measure
     * @return Total size in bytes
     */
    static size_t CalculateActualSize(const Chunk& chunk);
};
