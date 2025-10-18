#include "scenario_modifier.h"
#include "chunk_serializer.h"
#include "chunk_validator.h"
#include "base/byte_convertible.h"
#include <cstring>
#include <algorithm>

#pragma region Helper methods
//=========================================================
//=============== helper methods
//=========================================================
Chunk* ScenarioModifier::FindChunkRecursive(Chunk& chunk, ChunkType type) {
    if(chunk.header.chunk_type_identifier == type) {
        return &chunk;
    }
    
    for(auto& child : chunk.children) {
        Chunk* result = FindChunkRecursive(child, type);
        if(result != nullptr) {
            return result;
        }
    }
    
    return nullptr;
}

void ScenarioModifier::FindAllChunksRecursive(Chunk& chunk, ChunkType type, std::vector<Chunk*>& results) {
    if(chunk.header.chunk_type_identifier == type) {
        results.push_back(&chunk);
    }
    
    for(auto& child : chunk.children) {
        FindAllChunksRecursive(child, type, results);
    }
}

Chunk* ScenarioModifier::FindChunk(ChunkType type) {
    for(auto& chunk : rootChunks) {
        Chunk* result = FindChunkRecursive(chunk, type);
        if(result != nullptr) {
            return result;
        }
    }
    return nullptr;
}

std::vector<Chunk*> ScenarioModifier::FindAllChunks(ChunkType type) {
    std::vector<Chunk*> results;
    for(auto& chunk : rootChunks) {
        FindAllChunksRecursive(chunk, type, results);
    }
    return results;
}

#pragma endregion

#pragma region Player Modifications
//==========================================================
//========= Player Modifications
//==========================================================

bool ScenarioModifier::SetPlayerName(uint32_t playerIndex, const std::u16string& name) {
    auto chunks = FindAllChunks(ChunkType::PLAYER_PROPERTIES);
    
    for(auto* chunkPtr : chunks) {
        if(chunkPtr->data.size() < sizeof(PlayerPropertiesChunk0x6B)) {
            continue;
        }
        
        //parse the chunk
        PlayerPropertiesChunk0x6B props = ByteConvertible<PlayerPropertiesChunk0x6B>::from_bytes(chunkPtr->data);
        
        if(props.player_index == playerIndex) {
            //update the name (max 100 char16_t characters)
            size_t copyLen = std::min(name.length(), size_t(99));
            std::memcpy(props.player_name, name.c_str(), copyLen * sizeof(char16_t));
            props.player_name[copyLen] = 0; //null terminate
            
            //convert back to bytes
            auto bytes = props.to_bytes();
            chunkPtr->data.resize(bytes.size());
            std::memcpy(chunkPtr->data.data(), bytes.data(), bytes.size());
            
            return true;
        }
    }
    
    return false;
}

bool ScenarioModifier::SetPlayerColor(uint32_t playerIndex, uint8_t colorIndex) {
    auto chunks = FindAllChunks(ChunkType::PLAYER_PROPERTIES);
    
    for(auto* chunkPtr : chunks) {
        if(chunkPtr->data.size() < sizeof(PlayerPropertiesChunk0x6B)) {
            continue;
        }
        
        PlayerPropertiesChunk0x6B props = ByteConvertible<PlayerPropertiesChunk0x6B>::from_bytes(chunkPtr->data);
        
        if(props.player_index == playerIndex) {
            props.color_index = colorIndex;
            
            auto bytes = props.to_bytes();
            chunkPtr->data.resize(bytes.size());
            std::memcpy(chunkPtr->data.data(), bytes.data(), bytes.size());
            
            return true;
        }
    }
    
    return false;
}

bool ScenarioModifier::SetPlayerNation(uint32_t playerIndex, uint32_t nationIndex) {
    auto chunks = FindAllChunks(ChunkType::PLAYER_PROPERTIES);
    
    for(auto* chunkPtr : chunks) {
        if(chunkPtr->data.size() < sizeof(PlayerPropertiesChunk0x6B)) {
            continue;
        }
        
        PlayerPropertiesChunk0x6B props = ByteConvertible<PlayerPropertiesChunk0x6B>::from_bytes(chunkPtr->data);
        
        if(props.player_index == playerIndex) {
            props.nation_index = nationIndex;
            
            auto bytes = props.to_bytes();
            chunkPtr->data.resize(bytes.size());
            std::memcpy(chunkPtr->data.data(), bytes.data(), bytes.size());
            
            return true;
        }
    }
    
    return false;
}

bool ScenarioModifier::SetPlayerDifficulty(uint32_t playerIndex, uint32_t difficulty) {
    if(difficulty > 5) {
        //invalid difficulty
        return false;
    }
    
    auto chunks = FindAllChunks(ChunkType::PLAYER_PROPERTIES);
    
    for(auto* chunkPtr : chunks) {
        if(chunkPtr->data.size() < sizeof(PlayerPropertiesChunk0x6B)) {
            continue;
        }
        
        PlayerPropertiesChunk0x6B props = ByteConvertible<PlayerPropertiesChunk0x6B>::from_bytes(chunkPtr->data);
        
        if(props.player_index == playerIndex) {
            props.difficulty = difficulty;
            
            auto bytes = props.to_bytes();
            chunkPtr->data.resize(bytes.size());
            std::memcpy(chunkPtr->data.data(), bytes.data(), bytes.size());
            
            return true;
        }
    }
    
    return false;
}

bool ScenarioModifier::SetPlayerControl(uint32_t playerIndex, bool isHuman) {
    auto chunks = FindAllChunks(ChunkType::PLAYER_PROPERTIES);
    
    for(auto* chunkPtr : chunks) {
        if(chunkPtr->data.size() < sizeof(PlayerPropertiesChunk0x6B)) {
            continue;
        }
        
        PlayerPropertiesChunk0x6B props = ByteConvertible<PlayerPropertiesChunk0x6B>::from_bytes(chunkPtr->data);
        
        if(props.player_index == playerIndex) {
            props.control_field = isHuman ? 4 : 0;
            
            auto bytes = props.to_bytes();
            chunkPtr->data.resize(bytes.size());
            std::memcpy(chunkPtr->data.data(), bytes.data(), bytes.size());
            
            return true;
        }
    }
    
    return false;
}
#pragma endregion

#pragma region Map Modification
//==========================================================
//========= Map Modifications
//==========================================================

bool ScenarioModifier::GetMapDimensions(uint32_t& width, uint32_t& height) {
    Chunk* mapStructChunk = FindChunk(ChunkType::MAP_STRUCTURE);
    if(mapStructChunk == nullptr || mapStructChunk->data.size() < 12) {
        return false;
    }
    
    //MAP_STRUCTURE format: [total_tiles:4][width:4][height:4]
    std::memcpy(&width, mapStructChunk->data.data() + 4, sizeof(uint32_t));
    std::memcpy(&height, mapStructChunk->data.data() + 8, sizeof(uint32_t));
    
    return true;
}

bool ScenarioModifier::GetTileProperty(uint32_t x, uint32_t y, TilePropertyEntry& properties) {
    uint32_t width, height;
    if(!GetMapDimensions(width, height)) {
        return false;
    }
    
    if(x >= width || y >= height) {
        return false; //out of bounds
    }
    
    Chunk* tileChunk = FindChunk(ChunkType::TILE_PROPERTIES);
    if(tileChunk == nullptr) {
        return false;
    }
    
    size_t tileIndex = y * width + x;
    size_t offset = tileIndex * sizeof(TilePropertyEntry);
    
    if(offset + sizeof(TilePropertyEntry) > tileChunk->data.size()) {
        return false;
    }
    
    std::memcpy(&properties, tileChunk->data.data() + offset, sizeof(TilePropertyEntry));
    return true;
}

bool ScenarioModifier::SetTileProperty(uint32_t x, uint32_t y, const TilePropertyEntry& properties) {
    uint32_t width, height;
    if(!GetMapDimensions(width, height)) {
        return false;
    }
    
    if(x >= width || y >= height) {
        return false; //out of bounds
    }
    
    Chunk* tileChunk = FindChunk(ChunkType::TILE_PROPERTIES);
    if(tileChunk == nullptr) {
        return false;
    }
    
    size_t tileIndex = y * width + x;
    size_t offset = tileIndex * sizeof(TilePropertyEntry);
    
    if(offset + sizeof(TilePropertyEntry) > tileChunk->data.size()) {
        return false;
    }
    
    std::memcpy(tileChunk->data.data() + offset, &properties, sizeof(TilePropertyEntry));
    return true;
}

#pragma endregion

#pragma region Utility methods
//==========================================================
//========= Utility Methods
//==========================================================

uint32_t ScenarioModifier::GetPlayerCount() {
    Chunk* countChunk = FindChunk(ChunkType::ACTIVE_PLAYER_COUNT);
    if(countChunk == nullptr || countChunk->data.size() < sizeof(uint32_t)) {
        return 0;
    }
    
    uint32_t count;
    std::memcpy(&count, countChunk->data.data(), sizeof(uint32_t));
    return count;
}

bool ScenarioModifier::Validate() const {
    auto result = ChunkValidator::ValidateScenario(rootChunks);
    return result.valid;
}

void ScenarioModifier::UpdateAllChunkSizes() {
    for(auto& chunk : rootChunks) {
        ChunkSerializer::UpdateChunkSizes(chunk);
    }
}

#pragma endregion