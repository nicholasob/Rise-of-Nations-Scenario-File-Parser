#include "scenario_editor.h"
#include "scenario_parser.h"
#include "chunk_serializer.h"
#include "chunk_validator.h"
#include <fstream>
#include <iostream>

bool ScenarioEditor::LoadScenario(const std::string& filePath) {
    lastError.clear();
    //open the file
    std::ifstream file(filePath, std::ios::binary);
    if(!file) {
        lastError = "Failed to open file: " + filePath;
        std::cerr << lastError << std::endl;
        return false;
    }
    
    //read the file data
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(file)), {});
    file.close();
    
    if(fileData.empty()) {
        lastError = "File is empty: " + filePath;
        std::cerr << lastError << std::endl;
        return false;
    }
    
    //store original COMPRESSED data
    originalCompressedData = fileData;
    
    //decompress if needed
    std::vector<uint8_t> decompressed;
    if(GzipHelper::isGzip(fileData)) {
        if(!GzipHelper::decompressGzip(fileData, decompressed)) {
            lastError = "Failed to decompress file (gzip): " + filePath;
            std::cerr << lastError << std::endl;
            return false;
        }
    }
    else {
        decompressed = std::move(fileData);
    }
    
    //store the original decompressed data
    originalDecompressedData = decompressed;
    
    //parse the chunks
    try {
        size_t offset = 0;
        workingChunks.clear();
        
        while(offset < decompressed.size()) {
            Chunk chunk = ScenarioParser::ParseChunk(decompressed, offset);
            workingChunks.push_back(std::move(chunk));
        }
    }
    catch (const std::exception& e) {
        lastError = std::string("Error parsing file: ") + e.what();
        std::cerr << lastError << std::endl;
        workingChunks.clear();
        originalDecompressedData.clear();
        originalCompressedData.clear();
        return false;
    }
    
    //success!!
    loadedFilePath = filePath;
    modified = false;
    loaded = true;
    
    std::cout << "Successfully loaded scenario: " << filePath << std::endl;
    std::cout << "Parsed " << workingChunks.size() << " root chunks" << std::endl;
    
    return true;
}

ScenarioModifier ScenarioEditor::GetModifier() {
    if(!loaded) {
        throw std::runtime_error("No scenario loaded");
    }
    
    //mark the foöe as modified when creating a modifier
    modified = true;
    
    return ScenarioModifier(workingChunks);
}

bool ScenarioEditor::Save(const std::string& outputPath, bool compress) {
    if(!loaded) {
        lastError = "No scenario loaded!";
        std::cerr << lastError << std::endl;
        return false;
    }
    
    //update all the chunk sizes
    for(auto& chunk : workingChunks) {
        ChunkSerializer::UpdateChunkSizes(chunk);
    }
    
    //validate before saving - everything must be correct unless we want RoN to crash
    if(!Validate()) {
        lastError = "Validation failed, cannot save";
        std::cerr << lastError << std::endl;
        return false;
    }
    
    //serialize the chunks
    std::vector<uint8_t> serialized;
    try {
        serialized = ChunkSerializer::SerializeChunks(workingChunks);
    }
    catch (const std::exception& e) {
        lastError = std::string("Error serializing chunks: ") + e.what();
        std::cerr << lastError << std::endl;
        return false;
    }
    
    //compress if requested
    std::vector<uint8_t> outputData;
    if(compress) {
        if (!GzipHelper::compressGzip(serialized, outputData)) {
            lastError = "Failed to compress data";
            std::cerr << lastError << std::endl;
            return false;
        }
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
    
    if(!outFile) {
        std::cerr << "Error when writing to file: " << outputPath << std::endl;
        return false;
    }
    
    std::cout << "Successfully saved scenario to: " << outputPath << std::endl;
    std::cout << "Output size: " << outputData.size() << " bytes" 
              << (compress ? " (compressed)" : " (uncompressed)") << std::endl;
    
    // If saving to original path, update modified flag
    if (outputPath == loadedFilePath) {
        modified = false;
    }
    
    return true;
}

bool ScenarioEditor::Save(bool compress) {
    if(loadedFilePath.empty()) {
        lastError = "No file path specified";
        std::cerr << lastError << std::endl;
        return false;
    }
    return Save(loadedFilePath, compress);
}

void ScenarioEditor::Revert() {
    if(!loaded) {
        return;
    }
    
    //re-parse from the original decompressed data
    try {
        size_t offset = 0;
        workingChunks.clear();
        
        while(offset < originalDecompressedData.size()) {
            Chunk chunk = ScenarioParser::ParseChunk(originalDecompressedData, offset);
            workingChunks.push_back(std::move(chunk));
        }
        
        modified = false;
        std::cout << "Reverted all changes" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error when reverting: " << e.what() << std::endl;
    }
}

void ScenarioEditor::Close() {
    workingChunks.clear();
    originalDecompressedData.clear();
    originalCompressedData.clear();
    loadedFilePath.clear();
    modified = false;
    loaded = false;
}

bool ScenarioEditor::Validate() const {
    if(!loaded) {
        return false;
    }
    
    auto result = ChunkValidator::ValidateScenario(workingChunks);
    
    if(!result.valid) {
        std::cerr << "Validation errors:" << std::endl;
        for(const auto& error : result.errors) {
            std::cerr << "  ERROR: " << error << std::endl;
        }
    }
    
    if(!result.warnings.empty()) {
        std::cout << "Validation warnings:" << std::endl;
        for (const auto& warning : result.warnings) {
            std::cout << "  WARNING: " << warning << std::endl;
        }
    }
    
    return result.valid;
}
