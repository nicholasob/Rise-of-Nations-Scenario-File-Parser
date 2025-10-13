#pragma once
#include "data_structures.h"
#include <iostream>

class ScenarioParser {
public:
    //parse a single chunk from file data starting at a specific offset
    static Chunk parseChunk(const std::vector<uint8_t>& fileData, size_t& offset);
    
    //print the chunk tree structure for debugging
    static void printChunkTree(const Chunk& chunk, int depth = 0);
};
