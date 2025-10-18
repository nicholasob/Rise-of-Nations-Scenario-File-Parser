#pragma once
#include "data_structures.h"
#include "scenario_modifier.h"
#include "compression.h"
#include <string>
#include <vector>
#include <memory>

/**
 * loading, modifying, and saving scenario files.
 */
class ScenarioEditor {
private:
    std::string loadedFilePath;
    std::vector<uint8_t> originalCompressedData;
    std::vector<uint8_t> originalDecompressedData;
    std::vector<Chunk> workingChunks;
    bool modified;
    bool loaded;
    
public:
    ScenarioEditor() : modified(false), loaded(false) {}
    
    /**
     * Load a scenario file for editing
     * @param filePath path to the scenario file (.scx)
     * @return true if successful
     */
    bool LoadScenario(const std::string& filePath);
    
    /**
     * Get a modifier interface for making changes
     * @return ScenarioModifier instance
     */
    ScenarioModifier GetModifier();
    
    /**
     * Check if there are unsaved changes
     * @return true if scenario has been modified
     */
    bool HasUnsavedChanges() const { return modified; }
    
    /**
     * Check if a scenario is currently loaded
     * @return true if a scenario is loaded
     */
    bool IsLoaded() const { return loaded; }
    
    /**
     * Get the path of the currently loaded scenario
     * @return file path or empty string if none loaded
     */
    const std::string& GetLoadedPath() const { return loadedFilePath; }
    
    /**
     * Save modifications to a file
     * @param outputPath path to save to
     * @param compress whether to compress with gzip (default is true)
     * @return true if successful
     */
    bool Save(const std::string& outputPath, bool compress = true);
    
    /**
     * Save modifications back to the original file
     * @param compress whether to compress with gzip (default true)
     * @return true if successful
     */
    bool Save(bool compress = true);
    
    /**
     * revert all changes back to the original loaded state
     */
    void Revert();
    
    /**
     * unload the current scenario and clear all data
     */
    void Close();
    
    /**
     * Validate the current state of the scenario
     * @return true if valid
     */
    bool Validate() const;
    
    /**
     * get a read-only reference to the working chunks
     * @return Reference to chunk vector
     */
    const std::vector<Chunk>& getChunks() const { return workingChunks; }
    
    /**
     * mark the scenario as modified  - should be  called automatically by ScenarioModifier
     */
    void markModified() { modified = true; }
};
