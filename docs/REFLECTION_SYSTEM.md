# C++ Reflection System for Chunk Structures

This document explains how to use the reflection system to automatically sync struct definitions with GUI metadata.

## Problem Statement

Previously, when you modified a struct in `include/chunks/*.h`, you had to **manually update** `gui/chunk_metadata.cpp` with the same field information. This led to:
- Duplication of field definitions
- Easy-to-miss synchronization errors
- Maintenance burden

## Solution: Macro-Based Reflection

We've implemented a lightweight reflection system using C++17 that allows you to:
1. Define fields **once** in the struct
2. Add metadata inline with the struct
3. Automatically register with the GUI

## How to Use

### Step 1: Add Reflection to Your Struct

```cpp
#include "base/chunk_reflection.h"

#pragma pack(push, 1)
struct MyChunk : public ByteConvertible<MyChunk> {
    // 1. Define your fields as normal
    uint32_t player_index;
    uint32_t nation_index;
    char16_t player_name[100];

    // 2. Add reflection metadata AFTER the fields
    BEGIN_FIELD_DESCRIPTORS(MyChunk)
        DESCRIBE_FIELD(uint32_t, player_index, "Player slot (0-7)")
        DESCRIBE_FIELD(uint32_t, nation_index, "Nation the player is playing")
        DESCRIBE_FIELD(char16_t[100], player_name, "Player name (UTF-16)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
```

### Step 2: Register in GUI

In `gui/chunk_metadata.cpp`, instead of manual registration:

```cpp
// OLD WAY (manual):
ChunkInfo myChunk(ChunkType::MY_CHUNK, "MY_CHUNK", "Player", 236, colorPlayer);
myChunk.fields.push_back(FieldInfo("player_index", 0, 4, "uint32_t", "Player slot (0-7)"));
myChunk.fields.push_back(FieldInfo("nation_index", 4, 4, "uint32_t", "Nation the player is playing"));
myChunk.fields.push_back(FieldInfo("player_name", 8, 200, "char16_t[100]", "Player name (UTF-16)"));
registerChunk(myChunk);

// NEW WAY (automatic):
registerReflectedChunk<MyChunk>(
    ChunkType::MY_CHUNK,
    "MY_CHUNK",
    "Player",
    236,
    colorPlayer
);
```

That's it! The fields are automatically extracted from the struct.

## Benefits

### ✅ Single Source of Truth
Fields are defined **once** in the struct definition. No duplication.

### ✅ Compile-Time Safety
- `offsetof()` calculates correct field offsets automatically
- `sizeof()` gets the actual type sizes
- Type names are stringified using `#` operator
- Compiler errors if field names don't match

### ✅ Easy Maintenance
When you add/remove/modify a field:
1. Update the struct fields
2. Update the DESCRIBE_FIELD entries
3. That's it! GUI automatically updates

### ✅ No Runtime Overhead
All reflection happens at compile-time. Zero runtime cost.

## Examples

### Variable-Length Array Chunk

```cpp
#pragma pack(push, 1)
struct TechTreeUnitTypeNameChunk0x56 : public ByteConvertible<TechTreeUnitTypeNameChunk0x56> {
    char16_t unit_type_name[256];

    BEGIN_FIELD_DESCRIPTORS(TechTreeUnitTypeNameChunk0x56)
        DESCRIBE_FIELD(char16_t[256], unit_type_name, "Unit type name (UTF-16)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
```

GUI registration:
```cpp
registerReflectedChunk<TechTreeUnitTypeNameChunk0x56>(
    ChunkType::TECH_TREE_UNIT_TYPE_NAMES,
    "TECH_TREE_UNIT_TYPE_NAMES",
    "Tech Tree",
    512,
    colorMetadata
);
```

### Complex Struct with Many Fields

See `examples/reflected_chunk_example.h` for a full example with 21 fields.

## Migration Guide

To migrate existing structs to use reflection:

### Before:
```cpp
// include/chunks/unit_chunks.h
struct FormationChunk0x53 {
    uint32_t player_index;
    uint32_t formation_unit_id;
    // ... more fields
};

// gui/chunk_metadata.cpp (separate file!)
ChunkInfo formationProps(...);
formationProps.fields.push_back(FieldInfo("player_index", 0, 4, "uint32_t", "..."));
formationProps.fields.push_back(FieldInfo("formation_unit_id", 4, 4, "uint32_t", "..."));
registerChunk(formationProps);
```

### After:
```cpp
// include/chunks/unit_chunks.h
#include "base/chunk_reflection.h"

struct FormationChunk0x53 {
    uint32_t player_index;
    uint32_t formation_unit_id;
    // ... more fields

    BEGIN_FIELD_DESCRIPTORS(FormationChunk0x53)
        DESCRIBE_FIELD(uint32_t, player_index, "Player owning formation (0-7)")
        DESCRIBE_FIELD(uint32_t, formation_unit_id, "Formation unit pointer/ID")
        // ... more DESCRIBE_FIELD entries
    END_FIELD_DESCRIPTORS()
};

// gui/chunk_metadata.cpp (much simpler!)
registerReflectedChunk<FormationChunk0x53>(
    ChunkType::FORMATION_PROPERTIES,
    "FORMATION_PROPERTIES",
    "Units",
    580,
    colorUnits
);
```

## Technical Details

### How It Works

1. **`BEGIN_FIELD_DESCRIPTORS(StructName)`**: Creates a static method that returns field metadata
2. **`DESCRIBE_FIELD(Type, Name, Desc)`**: Uses `offsetof()` and `sizeof()` to extract field info
3. **`END_FIELD_DESCRIPTORS()`**: Closes the method
4. **`registerReflectedChunk<T>(...)`**: Calls `T::getFieldDescriptors()` and converts to `FieldInfo`

### Limitations

- Still requires macros (but much cleaner than manual duplication)
- Must list fields twice (once for declaration, once for DESCRIBE_FIELD)
- C++26 will have native reflection, making this obsolete

### Future: C++26 Native Reflection

When C++26 becomes available, this will work:

```cpp
template<typename T>
auto getFields() {
    constexpr auto members = std::meta::members_of(^T);
    // Automatically extract ALL field information!
}
```

No macros needed at all!

## Troubleshooting

### Compile Error: "offsetof() on non-standard-layout type"

Make sure your struct uses `#pragma pack(push, 1)` and is POD-compatible.

### Fields not appearing in GUI

1. Check that `BEGIN_FIELD_DESCRIPTORS` matches the struct name exactly
2. Verify you called `registerReflectedChunk<YourStruct>(...)` in `chunk_metadata.cpp`
3. Rebuild the project to regenerate reflection code

### Wrong offsets or sizes

The reflection system uses `offsetof()` and `sizeof()` which are compiler-accurate.
If offsets are wrong, the struct definition itself has issues (check padding, alignment).

## See Also

- `include/base/chunk_reflection.h` - Reflection macros and utilities
- `examples/reflected_chunk_example.h` - Complete working examples
- `gui/chunk_metadata.cpp` - Registration code

## Contributing

When adding new chunk types:
1. Define the struct with fields in `include/chunks/*.h`
2. Add `BEGIN_FIELD_DESCRIPTORS` ... `END_FIELD_DESCRIPTORS` to the struct
3. Call `registerReflectedChunk<YourStruct>(...)` in `gui/chunk_metadata.cpp`
4. Done! The GUI will automatically display all fields.
