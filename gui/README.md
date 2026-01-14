# RoN Scenario Viewer GUI

A Qt-based GUI application for viewing and editing Rise of Nations scenario files (.scx).

## Features

- **Hex Viewer**: Browse raw binary data with color-coded chunk highlighting
- **Chunk Tree**: Hierarchical display of scenario file structure
- **Properties Panel**: View decoded chunk fields with field names and types
- **Search**: Find hex patterns or text in the file
- **Hex Editing**: Directly edit bytes in the hex view
- **High-Level Editor**: Modify player properties (name, color, nation, difficulty, control)
- **Save**: Save changes with optional compression
- **UTF-16 Support**: Toggle between ASCII and UTF-16 text display

## Screenshots

(Screenshots would go here)

## Prerequisites

### Required Dependencies

1. **Qt 6 or Qt 5.15+**
   - Download from: https://www.qt.io/download-open-source-installer
   - During installation, select:
     - Qt 6.x (or Qt 5.15) for your compiler
     - CMake
     - Qt Creator (optional but recommended)

   **Windows (MSVC):**
   - Install Qt with MSVC 2019 or 2022 components
   - Example path: `C:\Qt\6.6.1\msvc2019_64`

   **Windows (MinGW):**
   - Install Qt with MinGW components
   - Example path: `C:\Qt\6.6.1\mingw_64`

   **Linux:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt6-base-dev qt6-tools-dev

   # Or for Qt5
   sudo apt-get install qtbase5-dev qttools5-dev
   ```

   **macOS:**
   ```bash
   brew install qt@6
   # Or
   brew install qt@5
   ```

2. **CMake 3.20+** (already installed at `C:\Program Files\CMake\bin\cmake.exe`)

3. **zlib** (already configured in your project)

4. **C++17 compiler**
   - MSVC 2019+ (Windows)
   - GCC 7+ (Linux)
   - Clang 5+ (macOS)

## Building the GUI

### Option 1: Using CMake (Command Line)

1. **Set Qt path** (if not in system PATH):

   **Windows:**
   ```bash
   set CMAKE_PREFIX_PATH=C:\Qt\6.6.1\msvc2019_64
   ```

   **Linux/macOS:**
   ```bash
   export CMAKE_PREFIX_PATH=/path/to/Qt/6.6.1/gcc_64
   ```

2. **Configure and build**:

   ```bash
   cd e:\RoN-Scenario-File-Parser
   mkdir build
   cd build
   cmake .. -DBUILD_GUI=ON
   cmake --build . --config Release
   ```

3. **Run the application**:

   ```bash
   # Windows
   .\Release\scenario_viewer_gui.exe

   # Linux/macOS
   ./scenario_viewer_gui
   ```

### Option 2: Using Qt Creator (Recommended for Windows)

1. Open Qt Creator
2. File → Open File or Project
3. Navigate to `e:\RoN-Scenario-File-Parser\CMakeLists.txt`
4. Configure the project with your Qt kit
5. Ensure `BUILD_GUI=ON` in CMake configuration
6. Build → Build Project
7. Run → Run (Ctrl+R)

### Option 3: Using Visual Studio (Windows)

1. Open Visual Studio
2. File → Open → CMake
3. Select `e:\RoN-Scenario-File-Parser\CMakeLists.txt`
4. Set Qt path in CMake settings:
   - CMake → CMake Settings
   - Add to CMake command arguments: `-DCMAKE_PREFIX_PATH=C:\Qt\6.6.1\msvc2019_64`
5. Build → Build All
6. Run `scenario_viewer_gui.exe`

## Troubleshooting

### Qt Not Found

**Error:** `Could not find a package configuration file provided by "Qt6"`

**Solution:**
- Install Qt as described above
- Set `CMAKE_PREFIX_PATH` to your Qt installation directory
- Example:
  ```bash
  cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/6.6.1/msvc2019_64
  ```

### Missing DLLs (Windows)

**Error:** Application fails to start with missing Qt DLL errors

**Solution:**
- Copy Qt DLLs to the executable directory, or
- Add Qt `bin` directory to PATH:
  ```bash
  set PATH=C:\Qt\6.6.1\msvc2019_64\bin;%PATH%
  ```
- Or use `windeployqt`:
  ```bash
  C:\Qt\6.6.1\msvc2019_64\bin\windeployqt.exe scenario_viewer_gui.exe
  ```

### Build Only CLI (Skip GUI)

If you don't want to build the GUI:

```bash
cmake .. -DBUILD_GUI=OFF
```

## Usage Guide

### Opening a Scenario File

1. File → Open (or Ctrl+O)
2. Select a `.scx` file
3. The file will be automatically decompressed and parsed
4. Chunk tree and hex view will populate

### Navigating the Hex View

- **Scroll**: Use mouse wheel or scrollbar
- **Click chunk in tree**: Hex view scrolls to that chunk and highlights it
- **Colors**: Different chunk categories have different background colors
  - Blue: Map data
  - Green: Player data
  - Orange: Units/Buildings
  - Purple: Advanced features
  - Gray: Metadata

### Searching

1. Enter search pattern in search bar:
   - **Hex bytes**: `0F 00 00 00` (space-separated hex values)
   - **Text**: Any ASCII text
2. Click "Next" or "Previous" to navigate matches
3. Results are highlighted in yellow

### Editing Hex Values

1. Click on a hex cell (columns 00-0F)
2. Type new hex value (2 digits)
3. Press Enter
4. The document will be marked as modified (*)

### High-Level Editing

1. Edit → High-Level Editor
2. **Players Tab**:
   - Modify player names (UTF-16 text)
   - Change colors (dropdown)
   - Set nations (dropdown)
   - Adjust difficulty
   - Toggle Human/Computer control
3. Click "Apply Changes"
4. Changes are reflected in hex view and chunk tree

### Saving

- **Save** (Ctrl+S): Overwrite original file
- **Save As** (Ctrl+Shift+S): Save to new location
- Files with `.scx` extension are automatically compressed (gzip)

### Properties Panel

1. Select a chunk in the tree
2. Switch to "Properties" tab
3. View decoded fields:
   - Field names
   - Data types
   - Byte offsets
   - Decoded values

## File Structure

```
gui/
├── CMakeLists.txt                      # Qt build configuration
├── main.cpp                            # Application entry point
├── main_window.h/cpp                   # Main application window
├── scenario_document.h/cpp             # Data model (load/save/modify)
├── hex_editor_widget.h/cpp             # Hex editor with search and editing
├── chunk_tree_widget.h/cpp             # Hierarchical chunk display
├── chunk_properties_widget.h/cpp       # Field decoder and display
├── chunk_metadata.h/cpp                # Chunk type metadata registry
├── high_level_editor_dialog.h/cpp      # Player/map modification dialog
└── README.md                           # This file
```

## Architecture

### Data Flow

```
User opens file
    ↓
ScenarioDocument::loadFile()
    ↓
Decompress (if gzipped) → Parse chunks → Store in memory
    ↓
Signal: dataLoaded()
    ↓
├── ChunkTreeWidget builds tree
├── HexEditorWidget displays bytes
└── Both widgets subscribe to chunk selection
    ↓
User selects chunk in tree
    ↓
Signal: chunkSelected(chunk)
    ↓
├── HexEditor scrolls to offset + highlights
└── PropertiesWidget decodes and displays fields
```

### Key Classes

- **ScenarioDocument**: Central data model, owns binary data and chunk tree
- **ChunkMetadata**: Singleton registry mapping ChunkType → field definitions
- **HexEditorWidget**: QTableWidget-based hex editor with color highlighting
- **ChunkTreeWidget**: QTreeWidget wrapper for hierarchical display
- **ChunkPropertiesWidget**: Decodes chunk data using metadata
- **HighLevelEditorDialog**: Uses ScenarioModifier APIs for safe edits

## Extending the Application

### Adding Field Metadata

Edit `chunk_metadata.cpp` to add more field definitions:

```cpp
ChunkInfo myChunk(ChunkType::MY_CHUNK, "MY_CHUNK", "Category", 100, colorMap);
myChunk.fields.push_back(FieldInfo("field_name", 0, 4, "uint32_t", "Description"));
myChunk.fields.push_back(FieldInfo("another_field", 4, 2, "uint16_t", "Another field"));
registerChunk(myChunk);
```

### Adding High-Level Operations

1. Add method to `ScenarioModifier` in core library
2. Add UI controls to `HighLevelEditorDialog`
3. Call modifier method in `applyChanges()`

### Custom Color Schemes

Modify colors in `ChunkMetadata` constructor:

```cpp
colorMap = QColor(173, 216, 230, 80);  // RGBA
```

## Known Limitations

1. **Large files**: Performance may degrade with files >100MB
2. **Undo/Redo**: Not yet implemented
3. **Field editing**: Properties panel is read-only (use hex editor or high-level dialog)
4. **Validation**: Limited real-time validation of edits

## Future Enhancements

- Undo/redo system
- Diff view for comparing two .scx files
- Export/import chunks as JSON
- Dark mode theme
- Bookmarks for offsets
- Memory-mapped file support
- Plugin system for custom decoders

## License

Same as parent project.

## Credits

Built using:
- Qt Framework (https://www.qt.io/)
- RoN Scenario Parser library (../include, ../src)
- zlib compression library
