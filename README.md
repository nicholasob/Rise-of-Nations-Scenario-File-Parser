# Rise of Nations Scenario File Parser

A C++ parser for reading, analyzing, and manipulating Rise of Nations scenario files (.scx). Its intended for extracting and working with scenario data including map information, player settings, units, buildings, triggers, and more. Still a work in progress!

## Features

- Parse Rise of Nations scenario (.scx) files
- Decompress and decrypt scenario data
- Extract detailed information about:
  - Map terrain and tiles
  - Player configurations
  - Units and buildings
  - Diplomatic relations
  - Triggers and conditions
  - Resources and goods
  - Visibility settings
- Support for most major chunk types used in RoN scenarios (work in progress)

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building the Project](#building-the-project)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

## Prerequisites

Before building the project, ensure you have the following installed:

- **CMake** 3.20 or higher
- **C++17 compatible compiler**
  - MSVC (Visual Studio 2017 or later) on Windows
  - GCC 7+ or Clang 5+ on Linux/macOS
- **zlib** library for compression support

### Installing Dependencies

#### Windows (with Conda)
```powershell
conda install -c conda-forge cmake zlib
```

#### Windows (with vcpkg)
```powershell
vcpkg install zlib:x64-windows
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install cmake build-essential zlib1g-dev
```

#### macOS
```bash
brew install cmake zlib
```

## Installation

Clone the repository:

```bash
git clone <repository-url>
cd RoN-Scenario-File-Parser
```

## Building the Project

### Configure the Build

Configure CMake and generate the build files:

**Windows (PowerShell):**
```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON
```

**Linux/macOS:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON
```

#### CMake Options

- `-DBUILD_CLI=ON` - Build the command-line interface (default: ON)
- `-DBUILD_TESTS=OFF` - Build unit tests (default: OFF)
- `-DCMAKE_BUILD_TYPE=Release` - Build type (Debug, Release, RelWithDebInfo, MinSizeRel)

### Build the Project

Once configured, compile the project:

```bash
cmake --build build
```

For faster builds on multi-core systems:

**Windows:**
```powershell
cmake --build build --parallel
```

**Linux/macOS:**
```bash
cmake --build build -j$(nproc)
```

### Clean Build

To perform a clean build:

**Windows:**
```powershell
Remove-Item -Recurse -Force build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON
cmake --build build
```

**Linux/macOS:**
```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON
cmake --build build
```

## Usage

### Running the Parser

After building, run the executable:

**Windows:**
```powershell
.\build\scenario_parser.exe
```

**Linux/macOS:**
```bash
./build/scenario_parser
```

### Command-Line Usage

The parser currently reads a input scenario file specified as a command-line argument and a optional output file:
scenario_parser <input_file> [output_file]

### Output

The parser will display detailed information about the scenario including:
- Scenario metadata
- Map dimensions and terrain data
- Player information
- Unit and building placements
- Trigger configurations
- Resource settings

## Project Structure

```
RoN-Scenario-File-Parser/
├── include/             # Header files
│   ├── base/            # Base classes and interfaces
│   ├── chunks/          # Chunk type definitions
│   └── common/          # Common utilities
├── chunk_parsers.cpp    # Chunk parsing implementations
├── chunk_types.cpp      # Chunk type definitions
├── compression.cpp      # Data compression/decompression
├── encryption.cpp       # Data encryption/decryption
├── scenario_parser.cpp  # Main scenario parser logic
├── main.cpp            # Entry point
├── CMakeLists.txt      # CMake build configuration
└── README.md           # This file
```

## Troubleshooting

### zlib Not Found

If CMake cannot find zlib:

1. **Using Conda:** Make sure you're in an activated Conda environment with zlib installed
2. **Manual specification:** Set the zlib path manually:
   ```bash
   cmake -S . -B build -DZLIB_ROOT=/path/to/zlib
   ```

### Build Errors

- Ensure you have a C++17 compatible compiler
- Try cleaning the build directory and reconfiguring
- Check that all dependencies are properly installed

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

Licensed under the MIT License

## Acknowledgments

- Rise of Nations by Big Huge Games