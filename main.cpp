#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cwchar>
#include <locale>
#include <codecvt>
#include <cstring>
#include <algorithm>
#include <iomanip>

#include "chunk_types.h"
#include "compression.h"
#include "data_structures.h"
#include "chunk_parsers.h"
#include "scenario_parser.h"
#include "scenario_editor.h"
#include "scenario_modifier.h"

size_t TILE_VISIBILITY_COUNT = SIZE_MAX;
//store coordinate counts for each group
std::vector<SpatialGroup> SPATIAL_GROUPS;

//Additional global variables for dynamic chunk parsing
size_t MAP_TOTAL_TILES = SIZE_MAX;
size_t SCALED_TOTAL_TILES = SIZE_MAX;
size_t TRIGGER_COUNT = SIZE_MAX;
size_t MAP_RESOURCES_COUNT = SIZE_MAX;
size_t PLAYER_COUNT = SIZE_MAX;
size_t RESOURCE_COUNT = SIZE_MAX;
size_t UNIT_TYPE_COUNT = SIZE_MAX;
size_t BUILDING_TYPE_COUNT = SIZE_MAX;
size_t FEATURE_COUNT = SIZE_MAX;
size_t LOCATION_COUNT = SIZE_MAX;
size_t ENTITY_COUNT = SIZE_MAX;

namespace {

const char* KnownLocationEntryName(uint32_t flagsAndId)
{
    switch (flagsAndId) {
        case LocationEntryChunk0x4b::TERRAIN_DECORATION_BUSHES:
            return "Bushes";
        case LocationEntryChunk0x4b::TERRAIN_DECORATION_ROCKS:
            return "Rocks";
        default:
            return nullptr;
    }
}

} // namespace

int RunParseMode(int argc, char* argv[]);
int RunModifyMode(int argc, char* argv[]);
void PrintUsageHelp();
void PrintModifyHelp();

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsageHelp();
        return 1;
    }
    
    std::string command = argv[1];
    
    //check if the user wants to modify the scenario file
    if (command == "--modify" || command == "-m") {
        return RunModifyMode(argc, argv);
    }
    
    //otherwise run in parse mode
    return RunParseMode(argc, argv);
}

void PrintUsageHelp() {
    std::cout << "Rise of Nations Scenario File Parser\n\n";
    std::cout << "Usage:\n";
    std::cout << "  Parse mode: scenario_parser <input.scx> [output.scx]\n";
    std::cout << "              Parses and displays scenario file structure\n\n";
    std::cout << "  Modify mode: scenario_parser --modify <input.scx> <output.scx> [options]\n";
    std::cout << "               Load, modify, and save scenario files\n\n";
    std::cout << "Modify Options:\n";
    std::cout << "  --player-name <index> <name>     Set player name (index 0-7)\n";
    std::cout << "  --player-color <index> <color>   Set player color (index 0-7, color 0-7)\n";
    std::cout << "  --player-nation <index> <nation> Set player nation ID\n";
    std::cout << "  --player-diff <index> <diff>     Set player difficulty (0=Easiest, 5=Toughest)\n";
    std::cout << "  --player-control <index> <type>  Set control (0=computer, 1=human)\n";
    std::cout << "  --no-compress                    Save without gzip compression\n";
    std::cout << "  --validate                       Validate all modifications before saving\n";
    std::cout << "\nExamples:\n";
    std::cout << "  scenario_parser scenario.scx\n";
    std::cout << "  scenario_parser --modify input.scx output.scx --player-name 0 \"My Player\"\n";
    std::cout << "  scenario_parser -m input.scx output.scx --player-color 1 3 --player-diff 1 4\n";
}

int RunParseMode(int argc, char* argv[]) {
    if(argc < 2 || argc > 3){
        std::cerr << "Usage: scenario_parser <input.scx> [output.scx]\n";
        return 1;
    }

    std::string output_path = "savefile.scx";
    bool savefile_set = false;
    if(argc == 3){
        output_path = argv[2];
        savefile_set = true;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if(!file){
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), {});
    file.close();

    //all scenario files are compressed using gzip
    std::vector<uint8_t> decompressed;
    if(GzipHelper::isGzip(data)){
        if(!GzipHelper::decompressGzip(data, decompressed)){
            std::cerr << "Failed to decompress GZip.\n";
            return 1;
        }
    }
    else{
        decompressed = std::move(data);
    }

    try{
        //parse the entire file as chunks
        size_t offset = 0;
        std::vector<Chunk> rootChunks;
        
        while(offset < decompressed.size()){
            Chunk chunk = ScenarioParser::ParseChunk(decompressed, offset);
            rootChunks.push_back(std::move(chunk));
        }
        
        //printing the chunk tree structure
        std::cout << "Scenario file structure:" << std::endl;
        for (const auto& chunk : rootChunks) {
            ScenarioParser::PrintChunkTree(chunk);
        }
        
        std::queue<const Chunk*> chunks;
        
        //Add all root chunks to the queue
        for(const auto& chunk : rootChunks){
            chunks.push(&chunk);
        }

        //each chunk contains sub-chunks
        while(!chunks.empty()){
            const Chunk* front = chunks.front();
            chunks.pop();

            //add children to queue for processing in a breadth-first approach
            for (const auto& ascending_chunk : front->children) {
                chunks.push(&ascending_chunk);
            }

            switch(front->header.chunk_type_identifier) {
                case ChunkType::METADATA:
                    if ((unsigned long int)front->data.size() >= sizeof(ScenarioHeader0x6C)) {
                        try {
                            ScenarioHeader0x6C metadata = ScenarioHeader0x6C::from_bytes(front->data);
                            //All scenario files I have processed uses the same magic bytes.
                            if(metadata.magic == 0xf00dcafe && metadata.version == 0xA){
                                std::cout << "Found valid scenario metadata!" << std::endl;
                                std::cout << "Version: " << metadata.version << std::endl;
                                std::cout << "Magic: 0x" << std::hex << metadata.magic << std::dec << std::endl;
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing metadata: " << e.what() << std::endl;
                        }
                    }
                    break;
                case ChunkType::MAP_STRUCTURE:
                    {
                        uint32_t total_tiles, width, height;
                        if (ChunkParsers::parseMapStructure(*front, total_tiles, width, height)) {
                            std::cout << "Map: " << width << "x" << height << " (" << total_tiles << " tiles)" << std::endl;
                            MAP_TOTAL_TILES = total_tiles; // Store for TILE_PROPERTIES parsing
                        }
                    }
                    break;
                    
                case ChunkType::MAP_NAME_DATA:
                    {
                        if ((unsigned long int)front->data.size() >= sizeof(MapNameChunk0x51)) {
                            try {
                                MapNameChunk0x51 mapname = MapNameChunk0x51::from_bytes(front->data);
                                std::cout << "Map name: " << ChunkParsers::ConvertChar16ToString(mapname.map_name, 100) << std::endl;
                            } catch (const std::exception& e) {
                                std::cerr << "Error parsing metadata: " << e.what() << std::endl;
                            }
                        }
                    }
                    break;
                    
                case ChunkType::RESOURCE_ENTRIES:
                    {
                        auto resources = ChunkParsers::DEPRECATED_parseResourceEntries(*front);
                        std::cout << "Found " << resources.size() << " resource entries:" << std::endl;
                        for (const auto& [index, amount] : resources) {
                            std::cout << "  Resource " << index << ": " << amount << std::endl;
                        }
                    }
                    break;
                    
                case ChunkType::PLAYER_PROPERTIES:
                    {
                        if ((unsigned long int)front->data.size() >= sizeof(PlayerPropertiesChunk0x6B)) {
                            try {
                                PlayerPropertiesChunk0x6B playerchunk = PlayerPropertiesChunk0x6B::from_bytes(front->data);
                                std::string flags = ChunkParsers::decodePlayerFlags(playerchunk.player_flags);
                                std::cout << flags << std::endl;
                            } catch (const std::exception& e) {
                                std::cerr << "Error parsing metadata: " << e.what() << std::endl;
                            }
                        }
                    }
                    break;
                case ChunkType::FORMATION_PROPERTIES:
                {
                    if ((unsigned long int)front->data.size() >= sizeof(FormationChunk0x53)) {
                        try {
                            FormationChunk0x53 formation = FormationChunk0x53::from_bytes(front->data);


                            std::wcout << L"Formation name: " << WideString::ConvertToWideString(formation.formation_unit_name) << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing metadata: " << e.what() << std::endl;
                        }
                    }
                }
                break;
                case ChunkType::VISIBILITY_DATA_COUNT:
                {
                    if ((unsigned long int)front->data.size() >= sizeof(VisibilityTriangleDataHeaderChunk0x64)) {
                        try {
                            VisibilityTriangleDataHeaderChunk0x64 visibilityCount = VisibilityTriangleDataHeaderChunk0x64::from_bytes(front->data);
                            TILE_VISIBILITY_COUNT = visibilityCount.triangle_count;

                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing metadata: " << e.what() << std::endl;
                        }
                    }
                }
                break;
                case ChunkType::MAP_LINKS:
                {
                    std::cout << "grd" << front->data.size() << std::endl;
                    try {
                        MapLinksChunk0x5b mapLinks = MapLinksChunk0x5b::from_bytes(front->data, front->data.size() / sizeof(uint16_t));
                        
                        std::cout << mapLinks.link_ids[0] << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing metadata: " << e.what() << std::endl;
                    }
                }
                break;
                case ChunkType::TILE_VISIBILITY:
                {
                    if(TILE_VISIBILITY_COUNT == SIZE_MAX)
                    {
                        std::cerr << "Error parsing tile visibility as dependent chunk 0x64 hasn't been parsed." << std::endl;
                        continue;
                    }

                    if ((unsigned long int)front->data.size() >= TILE_VISIBILITY_COUNT * sizeof(uint16_t)) {
                        try {
                            VisibilityTriangleIndicesChunk0x65 visibilityIndices = 
                                VisibilityTriangleIndicesChunk0x65::from_bytes(front->data, TILE_VISIBILITY_COUNT);
                            
                            std::cout << "Parsed " << visibilityIndices.indices.size() << " triangle indices. Example of triangle indice value: " << visibilityIndices.indices[0] << std::endl;
                            
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing visibility indices: " << e.what() << std::endl;
                        }
                    }
                }
                break;
                case ChunkType::SPATIAL_GROUP_COUNT: //0x5D
                {
                    std::cout << "Spatial Group Count chunk size: " << front->data.size() << std::endl;
                    
                    if ((unsigned long int)front->data.size() >= sizeof(AdvancedFeature2SpatialGroupCountChunk0x5D)) {
                        try {
                            AdvancedFeature2SpatialGroupCountChunk0x5D spatialGroupCount = AdvancedFeature2SpatialGroupCountChunk0x5D::from_bytes(front->data);

                            SpatialGroup spatialGroup(spatialGroupCount.group_count);
                            SPATIAL_GROUPS.push_back(spatialGroup);
                            
                            std::cout << "Found " << spatialGroupCount.group_count << " spatial groups" << std::endl;
                            
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing spatial group count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::GROUP_COORDINATE_COUNT: //0x5E
                {
                    std::cout << "Group Coordinate Count chunk size: " << front->data.size() << std::endl;
                    
                    if (SPATIAL_GROUPS.size() == 0) {
                        std::cerr << "Error: GROUP_COORDINATE_COUNT chunk found before SPATIAL_GROUP_COUNT chunk" << std::endl;
                        continue;
                    }
                    
                    if ((unsigned long int)front->data.size() >= sizeof(AdvancedFeature2GroupCoordinateCountChunk0x5E)) {
                        try {
                            AdvancedFeature2GroupCoordinateCountChunk0x5E coordCount = AdvancedFeature2GroupCoordinateCountChunk0x5E::from_bytes(front->data);
                            
                            //store this coordinate count for the corresponding 0x5F chunk
                            CoordinateGroup coordGroup(coordCount.coordinate_count);

                            SPATIAL_GROUPS[SPATIAL_GROUPS.size() - 1].CoordinateGroup.push_back(coordGroup);

                            std::cout << "Spatial group " << (SPATIAL_GROUPS.size() - 1) 
                                    << ", coordinate group " << SPATIAL_GROUPS.back().CoordinateGroup.size() - 1 
                                    << " has " 
                                    << coordCount.coordinate_count << " coordinates" << std::endl;
                            
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing group coordinate count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::COORDINATE_DATA: //0x5F
                {
                    std::cout << "Coordinate Data chunk size: " << front->data.size() << std::endl;
                    
                    if (SPATIAL_GROUPS.empty()) {
                        std::cerr << "Error: COORDINATE_DATA chunk found before SPATIAL_GROUP_COUNT chunk" << std::endl;
                        continue;
                    }
                    
                    if (SPATIAL_GROUPS.back().CoordinateGroup.empty()) {
                        std::cerr << "Error: COORDINATE_DATA chunk found before GROUP_COORDINATE_COUNT chunk" << std::endl;
                        continue;
                    }
                    
                    //get the expected count from the last coordinate group
                    CoordinateGroup& currentGroup = SPATIAL_GROUPS.back().CoordinateGroup.back();
                    
                    if ((unsigned long int)front->data.size() >= sizeof(AdvancedFeature2CoordinateArrayChunk0x5F)) {
                        try {
                            AdvancedFeature2CoordinateArrayChunk0x5F coord = 
                                AdvancedFeature2CoordinateArrayChunk0x5F::from_bytes(
                                    front->data
                                );

                            currentGroup.Coordinates.push_back(Coordinate(coord.x_processed, coord.y_processed));
                            
                            std::cout << "Parsed " << currentGroup.Coordinates.size() 
                                     << " coordinates for group " << (SPATIAL_GROUPS.size() - 1) 
                                     << ", coordinate group " << (SPATIAL_GROUPS.back().CoordinateGroup.size() - 1) << std::endl;
                            
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing coordinate array: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TILE_PROPERTIES: //0x10
                {
                    std::cout << "Tile Properties chunk size: " << front->data.size() << std::endl;
                    
                    if (MAP_TOTAL_TILES == SIZE_MAX) {
                        std::cerr << "Error: TILE_PROPERTIES chunk found before MAP_STRUCTURE chunk" << std::endl;
                        break;
                    }
                    
                    size_t expected_size = MAP_TOTAL_TILES * sizeof(TilePropertyEntry);
                    if ((unsigned long int)front->data.size() >= expected_size) {
                        try {
                            TilePropertiesChunk0x10 tileProperties = TilePropertiesChunk0x10::from_bytes(front->data, MAP_TOTAL_TILES);
                            std::cout << "Parsed " << tileProperties.tiles_entries.size() << " tile properties" << std::endl;
                            
                            //display first few tile properties as examples
                            size_t print_limit = std::min(static_cast<size_t>(5), tileProperties.tiles_entries.size());
                            for (size_t i = 0; i < print_limit; ++i) {
                                const auto& tile = tileProperties.tiles_entries[i];
                                std::cout << "  Tile[" << i << "]: flags=0x" << std::hex << (int)tile.terrain_flags << std::dec << std::endl;
                            }
                            
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing tile properties: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TERRAIN_LAYOUT_DATA: //0x11
                {
                    std::cout << "Terrain Layout Data chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(TerrainLayoutChunk0x11)) {
                        try {
                            TerrainLayoutChunk0x11 terrainLayout = TerrainLayoutChunk0x11::from_bytes(front->data);
                            std::cout << "Scaled total tiles: " << terrainLayout.scaled_total_tiles << std::endl;
                            std::cout << "Scaled width: " << terrainLayout.scaled_width << std::endl;
                            std::cout << "Scaled height: " << terrainLayout.scaled_height << std::endl;
                            SCALED_TOTAL_TILES = terrainLayout.scaled_total_tiles; //store for TERRAIN_TYPE parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing terrain layout: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TERRAIN_TYPE: //0x12
                {
                    std::cout << "Terrain Type chunk size: " << front->data.size() << std::endl;
                    
                    if (SCALED_TOTAL_TILES == SIZE_MAX) {
                        std::cerr << "Error: TERRAIN_TYPE chunk found before TERRAIN_LAYOUT_DATA chunk" << std::endl;
                        break;
                    }
                    
                    size_t expected_size = SCALED_TOTAL_TILES * sizeof(TerrainTypeFlags);
                    if ((unsigned long int)front->data.size() >= expected_size) {
                        try {
                            TerrainTypeChunk0x12 terrainType = TerrainTypeChunk0x12::from_bytes(front->data, SCALED_TOTAL_TILES);
                            std::cout << "Parsed " << terrainType.terrain_entries.size() << " terrain type entries" << std::endl;
                            
                            //show first few terrain types as examples
                            size_t print_limit = std::min(static_cast<size_t>(5), terrainType.terrain_entries.size());
                            for (size_t i = 0; i < print_limit; ++i) {
                                const auto& terrain = terrainType.terrain_entries[i];
                                std::cout << "  Terrain[" << i << "]: flags=0x" << std::hex << static_cast<uint16_t>(terrain) << std::dec << std::endl;
                            }
                            
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing terrain type: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TRIGGER_COUNT: //0x15
                {
                    std::cout << "Trigger Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(TriggerCountSubChunk0x15)) {
                        try {
                            TriggerCountSubChunk0x15 triggerCount = TriggerCountSubChunk0x15::from_bytes(front->data);
                            std::cout << "Active trigger count: " << triggerCount.active_count << std::endl;
                            TRIGGER_COUNT = triggerCount.active_count; //store for TRIGGER_ENTRIES parsing
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing trigger count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TRIGGER_ENTRIES: //0x16
                {
                    std::cout << "Trigger Entries chunk size: " << front->data.size() << std::endl;
                    if (TRIGGER_COUNT > 0) {
                        std::cout << "Parsing trigger entries for " << TRIGGER_COUNT << " triggers..." << std::endl;
                        try {
                            TriggerDataSubChunk0x16 triggerEntries = TriggerDataSubChunk0x16::from_bytes(front->data, TRIGGER_COUNT);
                            std::cout << "Successfully parsed trigger entries" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing trigger entries: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No trigger count available for parsing trigger entries" << std::endl;
                    }
                }
                break;

                case ChunkType::MAP_RESOURCES_COUNT: //0x17
                {
                    std::cout << "Map Resource Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(MapResourceCountSubChunk0x17)) {
                        try {
                            MapResourceCountSubChunk0x17 mapResourceCount = MapResourceCountSubChunk0x17::from_bytes(front->data);
                            std::cout << "Map resource count: " << mapResourceCount.map_resource_count << std::endl;
                            MAP_RESOURCES_COUNT = mapResourceCount.map_resource_count; //store for MAP_RESOURCE_ENTRIES parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing map_resources count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::MAP_RESOURCES_ENTRIES: //0x18
                {
                    std::cout << "Map Resource Entries chunk size: " << front->data.size() << std::endl;
                    if (MAP_RESOURCES_COUNT != SIZE_MAX) {
                        if (MAP_RESOURCES_COUNT == 0) {
                            std::cout << "No map resource entries present." << std::endl;
                            break;
                        }

                        std::cout << "Parsing map_resources entries for " << MAP_RESOURCES_COUNT << " map resources..." << std::endl;
                        try {
                            MapResourceDataSubChunk0x18 mapResourceEntries = MapResourceDataSubChunk0x18::from_bytes(front->data, MAP_RESOURCES_COUNT);
                            std::cout << "Successfully parsed map_resources entries" << std::endl;

                            const size_t printLimit = std::min(static_cast<size_t>(5), mapResourceEntries.map_resources.size());
                            for (size_t i = 0; i < printLimit; ++i) {
                                const auto& entry = mapResourceEntries.map_resources[i];
                                std::cout << "  MapResource[" << i << "]: "
                                          << ChunkParsers::ConvertChar16ToString(entry.name, 256)
                                          << " at (" << entry.position_x << ", " << entry.position_y << ")"
                                          << std::endl;
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing map_resources entries: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No map_resources count available for parsing map_resources entries" << std::endl;
                    }
                }
                break;

                case ChunkType::CITY_BUILDING_PROPERTIES: //0x1E
                {
                    std::cout << "City Building Properties chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(CityBuildingPropertiesChunk0x1e)) {
                        try {
                            CityBuildingPropertiesChunk0x1e building = CityBuildingPropertiesChunk0x1e::from_bytes(front->data);
                            std::cout << "Building player: " << building.player_index << std::endl;
                            std::cout << "Building index: " << building.building_index << std::endl;
                            std::wcout << L"City name: " << WideString::ConvertToWideString(building.city_name) << std::endl;
                            std::wcout << L"Building name: " << WideString::ConvertToWideString(building.building_name) << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing city building: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::GARRISON_COUNT: //0x21
                {
                    std::cout << "Garrison Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(GarrisonCountHeaderChunk0x21)) {
                        try {
                            GarrisonCountHeaderChunk0x21 garrisonCount = GarrisonCountHeaderChunk0x21::from_bytes(front->data);
                            std::cout << "Garrison unit count: " << garrisonCount.unit_count << std::endl;
                            std::cout << "Garrison type flag: " << garrisonCount.garrison_type_flag << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing garrison count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TECH_TREE_UNIT_TYPE_COUNT: //0x27
                {
                    std::cout << "Tech Tree Unit Type Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(TechTreeUnitTypeCountChunk0x27)) {
                        try {
                            TechTreeUnitTypeCountChunk0x27 unitTypeCount = TechTreeUnitTypeCountChunk0x27::from_bytes(front->data);
                            std::cout << "Library research entry count: " << unitTypeCount.unit_type_count << std::endl;
                            UNIT_TYPE_COUNT = unitTypeCount.unit_type_count; //store for Library research name parsing
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing unit type count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::ACTIVE_PLAYER_COUNT: //0x2A
                {
                    std::cout << "Active Player Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(PlayerCountChunk0x2a)) {
                        try {
                            PlayerCountChunk0x2a playerCount = PlayerCountChunk0x2a::from_bytes(front->data);
                            std::cout << "Active player count: " << playerCount.player_count << std::endl;
                            PLAYER_COUNT = playerCount.player_count; //Store for PLAYER_DETAILS parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing player count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::PLAYER_DETAILS: //0x2B
                {
                    std::cout << "Player Details chunk size: " << front->data.size() << std::endl;
                    if (PLAYER_COUNT != SIZE_MAX && PLAYER_COUNT > 0) {
                        std::cout << "Parsing player details for " << PLAYER_COUNT << " players..." << std::endl;
                        try {
                            PlayerDetailsChunk0x2b playerDetails = PlayerDetailsChunk0x2b::from_bytes(front->data, PLAYER_COUNT);
                            std::cout << "Successfully parsed player details" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing player details: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No player count available for parsing player details" << std::endl;
                    }
                }
                break;

                case ChunkType::RESOURCE_COUNT: //0x2D
                {
                    std::cout << "Resource Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(ResourceTypeCountChunk0x2d)) {
                        try {
                            ResourceTypeCountChunk0x2d resourceCount = ResourceTypeCountChunk0x2d::from_bytes(front->data);
                            std::cout << "Resource type count: " << resourceCount.resource_type_count << std::endl;
                            RESOURCE_COUNT = resourceCount.resource_type_count; //store for RESOURCE_DETAILS parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing resource count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::EDITOR_MAP_TERRAIN_DATA: //0x30
                {
                    std::cout << "Editor Map Terrain Data chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(EditorOrderGroupChunk0x30)) {
                        try {
                            EditorOrderGroupChunk0x30 editorGroup = EditorOrderGroupChunk0x30::from_bytes(front->data);
                            std::cout << "Group config: " << editorGroup.group_config << std::endl;
                            std::cout << "Units in group: " << editorGroup.units_in_group << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing editor group: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::EDITOR_RESOURCE_DATA: //0x32
                {
                    std::cout << "Editor Resource Data chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(EditorElementCountChunk0x32)) {
                        try {
                            EditorElementCountChunk0x32 elementCount = EditorElementCountChunk0x32::from_bytes(front->data);
                            std::cout << "Editor element count: " << elementCount.element_count << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing editor element count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::EDITOR_RESEARCH_PROGRESS: //0x33
                {
                    std::cout << "Editor Research Progress chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(EditorElementDataStructure0x33)) {
                        try {
                            EditorElementDataStructure0x33 elementData = EditorElementDataStructure0x33::from_bytes(front->data);
                            std::cout << "Element config: " << elementData.element_config << std::endl;
                            std::cout << "Element type: " << elementData.element_type << std::endl;
                            std::cout << "Coordinates: (" << elementData.x_coordinate << ", " << elementData.y_coordinate << ")" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing editor element data: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::FEATURE_COUNT: //0x46
                {
                    std::cout << "Feature Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(AdvancedFeature1MountainFeaturesChunk0x46)) {
                        try {
                            AdvancedFeature1MountainFeaturesChunk0x46 featureCount = AdvancedFeature1MountainFeaturesChunk0x46::from_bytes(front->data);
                            std::cout << "Mountain feature count: " << featureCount.feature_count << std::endl;
                            FEATURE_COUNT = featureCount.feature_count; //store for FEATURE_DATA parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing feature count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::FEATURE_DATA: //0x47
                {
                    std::cout << "Feature Data chunk size: " << front->data.size() << std::endl;
                    if (FEATURE_COUNT != SIZE_MAX && FEATURE_COUNT > 0) {
                        std::cout << "Parsing feature data for " << FEATURE_COUNT << " features..." << std::endl;
                        try {
                            AdvancedFeature1MountainFeatureChunk0x47 featureData = AdvancedFeature1MountainFeatureChunk0x47::from_bytes(front->data, FEATURE_COUNT);
                            std::cout << "Successfully parsed feature data" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing feature data: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No feature count available for parsing feature data" << std::endl;
                    }
                }
                break;

                case ChunkType::LOCATION_COUNT: //0x4A
                {
                    std::cout << "Location Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(AdvancedFeature4Chunk0x4a)) {
                        try {
                            AdvancedFeature4Chunk0x4a locationCount = AdvancedFeature4Chunk0x4a::from_bytes(front->data);
                            std::cout << "Location entry count: " << locationCount.location_count << std::endl;
                            LOCATION_COUNT = locationCount.location_count; //store for LOCATION_DATA parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing location count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::LOCATION_DATA: //0x4B
                {
                    std::cout << "Location Data chunk size: " << front->data.size() << std::endl;
                    if (LOCATION_COUNT != SIZE_MAX) {
                        if (LOCATION_COUNT == 0) {
                            std::cout << "No location entries present." << std::endl;
                            break;
                        }

                        std::cout << "Parsing location data for " << LOCATION_COUNT << " location entries..." << std::endl;
                        try {
                            AdvancedFeature4Chunk0x4b locationData = AdvancedFeature4Chunk0x4b::from_bytes(front->data, LOCATION_COUNT);
                            std::cout << "Successfully parsed location data" << std::endl;

                            const size_t printLimit = std::min(static_cast<size_t>(5), locationData.location_entries.size());
                            for (size_t i = 0; i < printLimit; ++i) {
                                const auto& entry = locationData.location_entries[i];
                                std::cout << "  Location[" << i << "]: (" << entry.x_coordinate << ", " << entry.y_coordinate << ")";

                                if (const char* decorationName = KnownLocationEntryName(entry.flags_and_id)) {
                                    std::cout << " flags_and_id=0x" << std::uppercase << std::hex
                                              << std::setw(8) << std::setfill('0') << entry.flags_and_id << std::nouppercase << std::dec
                                              << " (" << decorationName << ")";
                                } else {
                                    std::cout << " flags_and_id=0x" << std::uppercase << std::hex
                                              << std::setw(8) << std::setfill('0') << entry.flags_and_id << std::nouppercase << std::dec;
                                }

                                std::cout << std::endl;
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing location data: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No location count available for parsing location data" << std::endl;
                    }
                }
                break;

                case ChunkType::TECH_TREE_BUILDING_TYPE_COUNT: //0x4D
                {
                    std::cout << "Tech Tree Building Type Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(TechTreeBuildingTypeCountChunk0x4d)) {
                        try {
                            TechTreeBuildingTypeCountChunk0x4d buildingTypeCount = TechTreeBuildingTypeCountChunk0x4d::from_bytes(front->data);
                            std::cout << "Building type count: " << buildingTypeCount.building_type_count << std::endl;
                            BUILDING_TYPE_COUNT = buildingTypeCount.building_type_count; //store for building names parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing building type count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TECH_TREE_BUILDING_TYPE_NAMES: //0x4E
                {
                    std::cout << "Tech Tree Building Type Names chunk size: " << front->data.size() << std::endl;
                    if (BUILDING_TYPE_COUNT > 0) {
                        std::cout << "Parsing building type names for " << BUILDING_TYPE_COUNT << " types..." << std::endl;
                        try {
                            TechTreeBuildingTypeNameChunk0x4e buildingNames = TechTreeBuildingTypeNameChunk0x4e::from_bytes(front->data, BUILDING_TYPE_COUNT);
                            std::cout << "Successfully parsed building type names" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing building type names: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No building type count available for parsing building names" << std::endl;
                    }
                }
                break;

                case ChunkType::BUILDING_GROUP_PROPERTIES: //0x54
                {
                    std::cout << "Building Group Properties chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(BuildingPropertiesChunk0x54)) {
                        try {
                            BuildingPropertiesChunk0x54 building = BuildingPropertiesChunk0x54::from_bytes(front->data);
                            std::cout << "Building player: " << building.player_index << std::endl;
                            std::cout << "Entity index: " << building.entity_index << std::endl;
                            std::wcout << L"Entity name: " << WideString::ConvertToWideString(building.entity_name) << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing building properties: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::TECH_TREE_UNIT_TYPE_NAMES: //0x56
                {
                    std::cout << "Tech Tree Unit Type Names chunk size: " << front->data.size() << std::endl;
                    if (UNIT_TYPE_COUNT > 0) {
                        std::cout << "Parsing Library research names for " << UNIT_TYPE_COUNT << " entries..." << std::endl;
                        try {
                            TechTreeUnitTypeNameChunk0x56 unitNames = TechTreeUnitTypeNameChunk0x56::from_bytes(front->data, UNIT_TYPE_COUNT);
                            std::cout << "Successfully parsed Library research names" << std::endl;
                            std::cout << "Printing all names found: " << std::endl;
                            for(auto unit : unitNames.unit_names) {
                                std::cout << ChunkParsers::ConvertChar16ToString(unit.name, 256) << std::endl;
                            }
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing unit type names: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No Library research count available for parsing names" << std::endl;
                    }
                }
                break;

                case ChunkType::OBJECT_GROUPS: //0x5A
                {
                    std::cout << "Object Groups chunk size: " << front->data.size() << std::endl;
                    try {
                        ObjectGroupsChunk0x5A objectGroups = ObjectGroupsChunk0x5A::from_bytes(front->data);
                        std::cout << "Object group count: " << objectGroups.group_count << std::endl;
                        for (size_t i = 0; i < objectGroups.object_groups.size() && i < 5; ++i) {
                            const auto& group = objectGroups.object_groups[i];
                            std::cout << "  Group " << i << ": " << group.object_count << " objects" << std::endl;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing object groups: " << e.what() << std::endl;
                    }
                }
                break;

                case ChunkType::ENTITY_COUNT: //0x61
                {
                    std::cout << "Entity Count chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(AdvancedFeature3Chunk0x61)) {
                        try {
                            AdvancedFeature3Chunk0x61 entityCount = AdvancedFeature3Chunk0x61::from_bytes(front->data);
                            std::cout << "Entity count: " << entityCount.entry_count << std::endl;
                            ENTITY_COUNT = entityCount.entry_count; //store for ENTITY_DATA parsing later on
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing entity count: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::ENTITY_DATA: //0x62
                {
                    std::cout << "Entity Data chunk size: " << front->data.size() << std::endl;
                    if (ENTITY_COUNT != SIZE_MAX && ENTITY_COUNT > 0) {
                        std::cout << "Parsing entity data for " << ENTITY_COUNT << " entities..." << std::endl;
                        try {
                            AdvancedFeature3CoordinateArrayChunk0x62 entityData = AdvancedFeature3CoordinateArrayChunk0x62::from_bytes(front->data, ENTITY_COUNT);
                            std::cout << "Successfully parsed entity data" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing entity data: " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "Warning: No entity count available for parsing entity data" << std::endl;
                    }
                }
                break;

                case ChunkType::MAP_METADATA_DATA: //0x69
                {
                    std::cout << "Map Metadata Data chunk size: " << front->data.size() << std::endl;
                    if ((unsigned long int)front->data.size() >= sizeof(MapMetadataChunk0x69)) {
                        try {
                            MapMetadataChunk0x69 metadata = MapMetadataChunk0x69::from_bytes(front->data);
                            std::cout << "Tile count: " << metadata.tile_count << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing map metadata: " << e.what() << std::endl;
                        }
                    }
                }
                break;

                case ChunkType::EXTENDED_MAP_DATA: //0x6A
                {
                    std::cout << "Extended Map Data chunk size: " << front->data.size() << std::endl;
                    try {
                        ExtendedMapData0x6a extendedData = ExtendedMapData0x6a::from_bytes(front->data, front->data.size());
                        std::cout << "Parsed " << extendedData.extended_map_data.size() << " bytes of extended map data" << std::endl;
                        
                        //show first few bytes
                        size_t print_limit = std::min(static_cast<size_t>(16), extendedData.extended_map_data.size());
                        std::cout << "First " << print_limit << " bytes: ";
                        for (size_t i = 0; i < print_limit; ++i) {
                            std::cout << "0x" << std::hex << (int)extendedData.extended_map_data[i] << " ";
                        }
                        std::cout << std::dec << std::endl;
                        
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing extended map data: " << e.what() << std::endl;
                    }
                }
                break;

                case ChunkType::VARIABLE_DATA_CHUNK: //0x08
                {
                    std::cout << "Variable Data Chunk size: " << front->data.size() << std::endl;
                    
                    try {
                        VariableDataChunk0x08 varChunk = VariableDataChunk0x08::parse_from_chunk(*front);
                        
                        std::cout << "Primary value: " << varChunk.primary_value << std::endl;
                        if (varChunk.has_flags_field) {
                            std::cout << "Flags: 0x" << std::hex << varChunk.flags_optional << std::dec << std::endl;
                        }
                        std::cout << "Element count: " << varChunk.element_count << std::endl;
                        
                        //show first few elements
                        size_t print_limit = std::min(static_cast<size_t>(10), varChunk.elements.size());
                        for (size_t i = 0; i < print_limit; ++i) {
                            std::cout << "  Element[" << i << "]: 0x" << std::hex << varChunk.elements[i] << std::dec << std::endl;
                        }
                        
                        if (varChunk.elements.size() > print_limit) {
                            std::cout << "  ... and " << (varChunk.elements.size() - print_limit) 
                                    << " more elements" << std::endl;
                        }
                        
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing Variable Data Chunk: " << e.what() << std::endl;
                    }
                }
                break;

                default:
                    break;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing file: " << e.what() << std::endl;
        return 1;
    }

    if(savefile_set)
    {
        std::ofstream out(output_path, std::ios::binary);
        if (!out) {
            std::cerr << "Failed to open output file for writing.\n";
            return 1;
        }
        out.write(reinterpret_cast<const char*>(decompressed.data()), static_cast<std::streamsize>(decompressed.size()));
        out.close();
    }
    
    return 0;
}

int RunModifyMode(int argc, char* argv[]) {
    //minimum args: program --modify input output
    if(argc < 4) {
        std::cerr << "Error: Modify mode requires input and output files\n";
        std::cerr << "Usage: scenario_parser --modify <input.scx> <output.scx> [options]\n";
        return 1;
    }
    
    std::string inputPath = argv[2];
    std::string outputPath = argv[3];
    bool compress = true;
    bool validateOnly = false;
    
    //create editor and load scenario
    ScenarioEditor editor;
    std::cout << "Loading scenario file: " << inputPath << std::endl;
    
    if(!editor.LoadScenario(inputPath)) {
        std::cerr << "Failed to load scenario file\n";
        return 1;
    }
    
    //get modifier interface
    ScenarioModifier modifier = editor.GetModifier();
    
    //process modification options
    for(int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        
        if(arg == "--no-compress") {
            compress = false;
            std::cout << "Compression disabled\n";
        }
        else if(arg == "--validate") {
            validateOnly = true;
        }
        else if(arg == "--player-name" && i + 2 < argc) {
            uint32_t playerIndex = std::atoi(argv[++i]);
            std::string name = argv[++i];
            
            //convert to UTF-16
            std::u16string u16name;
            for(char c : name) {
                u16name.push_back(static_cast<char16_t>(c));
            }
            
            if(modifier.SetPlayerName(playerIndex, u16name)) {
                std::cout << "Set player " << playerIndex << " name to: " << name << std::endl;
            }
            else {
                std::cerr << "Failed to set player " << playerIndex << " name\n";
            }
        }
        else if(arg == "--player-color" && i + 2 < argc) {
            uint32_t playerIndex = std::atoi(argv[++i]);
            uint8_t colorIndex = static_cast<uint8_t>(std::atoi(argv[++i]));
            
            if(modifier.SetPlayerColor(playerIndex, colorIndex)) {
                std::cout << "Set player " << playerIndex << " color to: " << static_cast<int>(colorIndex) << std::endl;
            }
            else {
                std::cerr << "Failed to set player " << playerIndex << " color\n";
            }
        }
        else if(arg == "--player-nation" && i + 2 < argc) {
            uint32_t playerIndex = std::atoi(argv[++i]);
            uint32_t nationIndex = std::atoi(argv[++i]);
            
            if(modifier.SetPlayerNation(playerIndex, nationIndex)) {
                std::cout << "Set player " << playerIndex << " nation to: " << nationIndex << std::endl;
            }
            else {
                std::cerr << "Failed to set player " << playerIndex << " nation\n";
            }
        }
        else if (arg == "--player-diff" && i + 2 < argc) {
            uint32_t playerIndex = std::atoi(argv[++i]);
            uint32_t difficulty = std::atoi(argv[++i]);
            
            if (modifier.SetPlayerDifficulty(playerIndex, difficulty)) {
                std::cout << "Set player " << playerIndex << " difficulty to: " << difficulty << std::endl;
            } else {
                std::cerr << "Failed to set player " << playerIndex << " difficulty\n";
            }
        }
        else if (arg == "--player-control" && i + 2 < argc) {
            uint32_t playerIndex = std::atoi(argv[++i]);
            bool isHuman = std::atoi(argv[++i]) != 0;
            
            if (modifier.SetPlayerControl(playerIndex, isHuman)) {
                std::cout << "Set player " << playerIndex << " control to: " 
                         << (isHuman ? "human" : "computer") << std::endl;
            } else {
                std::cerr << "Failed to set player " << playerIndex << " control\n";
            }
        }
        else if (arg.substr(0, 2) == "--") {
            std::cerr << "Unknown option: " << arg << std::endl;
        }
    }
    
    //update chunk sizes after modifications
    modifier.UpdateAllChunkSizes();
    
    //validate
    std::cout << "\nValidating modifications...\n";
    if(!modifier.Validate()) {
        std::cerr << "Validation failed!\n";
        if(!validateOnly) {
            std::cerr << "Aborting save operation\n";
            return 1;
        }
    }
    else {
        std::cout << "Validation passed!\n";
    }
    
    if(validateOnly) {
        std::cout << "Validate-only mode, not saving\n";
        return 0;
    }
    
    //save the modified scenario
    std::cout << "\nSaving modified scenario to: " << outputPath << std::endl;
    if (editor.Save(outputPath, compress)) {
        std::cout << "Successfully saved modified scenario!\n";
        return 0;
    }
    else {
        std::cerr << "Failed to save modified scenario\n";
        return 1;
    }
}
