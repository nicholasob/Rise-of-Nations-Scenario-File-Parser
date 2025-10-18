#pragma once
#include "data_structures.h"
#include "chunk_types.h"
#include "chunks/player_chunks.h"
#include "chunks/map_chunks.h"
#include "chunks/resource_chunks.h"
#include <vector>
#include <string>
#include <cstdint>

/**
 * methods for modifying common scenario properties like player settings, map data, resources, etc.
 */
class ScenarioModifier {
private:
    std::vector<Chunk>& rootChunks;
    
    /**
     * Find the first chunk matching the given type
     * @param type the chunk type to find
     * @return pointer to chunk or nullptr if not found
     */
    Chunk* FindChunk(ChunkType type);
    
    /**
     * Find all chunks matching the given type
     * @param type the chunk type to find
     * @return vector of pointers to matching chunks
     */
    std::vector<Chunk*> FindAllChunks(ChunkType type);
    
    /**
     * Search recursively for a chunk of given type
     * @param chunk the chunk to search in
     * @param type the type to search for
     * @return pointer to chunk or nullptr
     */
    Chunk* FindChunkRecursive(Chunk& chunk, ChunkType type);
    
    /**
     * Search recursively for all chunks of given type
     * @param chunk the chunk to search in
     * @param type the type to search for
     * @param results vector to accumulate results
     */
    void FindAllChunksRecursive(Chunk& chunk, ChunkType type, std::vector<Chunk*>& results);

public:
    ScenarioModifier(std::vector<Chunk>& chunks) : rootChunks(chunks) {}
    
    /**
     * Set a player's name
     * @param playerIndex player slot (0-7)
     * @param name new player name (UTF-16)
     * @return true if successful
     */
    bool SetPlayerName(uint32_t playerIndex, const std::u16string& name);
    
    /**
     * Set a player's color
     * @param playerIndex player slot (0-7)
     * @param colorIndex color index (0=red, 1=blue, etc.)
     * @return true if successful
     */
    bool SetPlayerColor(uint32_t playerIndex, uint8_t colorIndex);
    
    /**
     * Set a player's nation
     * @param playerIndex player slot (0-7)
     * @param nationIndex nation ID
     * @return true if successful
     */
    bool SetPlayerNation(uint32_t playerIndex, uint32_t nationIndex);
    
    /**
     * Set a player's difficulty level
     * @param playerIndex player slot (0-7)
     * @param difficulty difficulty level (0=Easiest, 5=Toughest)
     * @return true if successful
     */
    bool SetPlayerDifficulty(uint32_t playerIndex, uint32_t difficulty);
    
    /**
     * Set whether a player is human or computer controlled
     * @param playerIndex player slot (0-7)
     * @param isHuman true for human, false for computer
     * @return true if successful
     */
    bool SetPlayerControl(uint32_t playerIndex, bool isHuman);
    
    /**
     * Get map dimensions
     * @param width output parameter for map width
     * @param height output parameter for map height
     * @return true if successful
     */
    bool GetMapDimensions(uint32_t& width, uint32_t& height);
    
    /**
     * Set terrain properties for a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @param properties tile properties
     * @return true if successful
     */
    bool SetTileProperty(uint32_t x, uint32_t y, const TilePropertyEntry& properties);
    
    /**
     * Get terrain properties for a specific tile
     * @param x X coordinate
     * @param y Y coordinate
     * @param properties output parameter for tile properties
     * @return true if successful
     */
    bool GetTileProperty(uint32_t x, uint32_t y, TilePropertyEntry& properties);
    
    /**
     * Get the number of players in the scenario
     * @return player count or 0 if not found
     */
    uint32_t GetPlayerCount();
    
    /**
     * Check if modifications are valid
     * @return true if all modifications maintain file integrity
     */
    bool Validate() const;
    
    /**
     * Update all chunk sizes after modifications
     */
    void UpdateAllChunkSizes();
};
