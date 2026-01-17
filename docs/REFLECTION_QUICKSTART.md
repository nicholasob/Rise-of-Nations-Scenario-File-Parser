# Reflection System - Quick Start Guide

## TL;DR

**Before:** Modify struct → manually update GUI metadata (error-prone)
**After:** Modify struct → add reflection macros → GUI auto-updates! ✨

## 30-Second Example

```cpp
// 1. Add this include
#include "base/chunk_reflection.h"

// 2. Define your struct
#pragma pack(push, 1)
struct MyChunk : public ByteConvertible<MyChunk> {
    uint32_t player_id;
    char16_t name[256];

    // 3. Add reflection (one line per field)
    BEGIN_FIELD_DESCRIPTORS(MyChunk)
        DESCRIBE_FIELD(uint32_t, player_id, "Player ID (0-7)")
        DESCRIBE_FIELD(char16_t[256], name, "Player name (UTF-16)")
    END_FIELD_DESCRIPTORS()
};
#pragma pack(pop)
```

```cpp
// 4. In gui/chunk_metadata.cpp - one function call:
registerReflectedChunk<MyChunk>(
    ChunkType::MY_CHUNK,
    "MY_CHUNK",
    "Player",
    sizeof(MyChunk),
    colorPlayer
);
```

**Done!** The GUI now displays:
```
Field Name    | Type            | Offset | Value
------------------------------------------------
player_id     | uint32_t        | 0      | 3
name          | char16_t[256]   | 4      | Player Red
```

## Why Use Reflection?

### Without Reflection ❌
```cpp
// struct definition in unit_chunks.h
struct FormationChunk {
    uint32_t player_index;
    uint32_t formation_id;
};

// Manually duplicate in chunk_metadata.cpp (error-prone!)
formationInfo.fields.push_back(FieldInfo("player_index", 0, 4, "uint32_t", "..."));
formationInfo.fields.push_back(FieldInfo("formation_id", 4, 4, "uint32_t", "..."));

// Add a field? Update TWO places. Forget? GUI breaks silently.
```

### With Reflection ✅
```cpp
// struct definition with inline metadata
struct FormationChunk {
    uint32_t player_index;
    uint32_t formation_id;

    BEGIN_FIELD_DESCRIPTORS(FormationChunk)
        DESCRIBE_FIELD(uint32_t, player_index, "Player index (0-7)")
        DESCRIBE_FIELD(uint32_t, formation_id, "Formation ID")
    END_FIELD_DESCRIPTORS()
};

// One-line registration
registerReflectedChunk<FormationChunk>(...);

// Add a field? Just add to struct + add DESCRIBE_FIELD. Done!
```

## Migration Checklist

Migrating an existing struct? Follow these steps:

- [ ] Add `#include "base/chunk_reflection.h"` to your chunk header
- [ ] Add `BEGIN_FIELD_DESCRIPTORS(YourStruct)` after the fields
- [ ] Add `DESCRIBE_FIELD(Type, name, "Description")` for each field
- [ ] Add `END_FIELD_DESCRIPTORS()` at the end
- [ ] In `gui/chunk_metadata.cpp`: replace manual registration with `registerReflectedChunk<YourStruct>(...)`
- [ ] Delete the old manual field definitions
- [ ] Build and test!

## Common Patterns

### Single Field
```cpp
struct SimpleChunk {
    uint32_t count;

    BEGIN_FIELD_DESCRIPTORS(SimpleChunk)
        DESCRIBE_FIELD(uint32_t, count, "Element count")
    END_FIELD_DESCRIPTORS()
};
```

### Array Field
```cpp
struct ArrayChunk {
    char16_t name[256];

    BEGIN_FIELD_DESCRIPTORS(ArrayChunk)
        DESCRIBE_FIELD(char16_t[256], name, "Name (UTF-16)")
    END_FIELD_DESCRIPTORS()
};
```

### Many Fields
```cpp
struct ComplexChunk {
    uint32_t field1;
    uint32_t field2;
    char16_t name[100];
    uint8_t flags;

    BEGIN_FIELD_DESCRIPTORS(ComplexChunk)
        DESCRIBE_FIELD(uint32_t, field1, "Description 1")
        DESCRIBE_FIELD(uint32_t, field2, "Description 2")
        DESCRIBE_FIELD(char16_t[100], name, "Name")
        DESCRIBE_FIELD(uint8_t, flags, "Status flags")
    END_FIELD_DESCRIPTORS()
};
```

## Pro Tips

### 💡 Tip 1: Copy-Paste from Struct
```cpp
// Start with your fields:
uint32_t player_index;
uint32_t nation_index;

// Copy-paste and wrap in DESCRIBE_FIELD:
DESCRIBE_FIELD(uint32_t, player_index, "TODO")
DESCRIBE_FIELD(uint32_t, nation_index, "TODO")

// Then add descriptions
```

### 💡 Tip 2: Use sizeof() in Registration
```cpp
// Instead of hardcoding size:
registerReflectedChunk<MyChunk>(..., 512, ...);

// Use sizeof():
registerReflectedChunk<MyChunk>(..., sizeof(MyChunk), ...);
```

### 💡 Tip 3: Compiler Catches Errors
```cpp
DESCRIBE_FIELD(uint32_t, playername, "...")  // Typo!
//                       ^^^^^^^^^^
// Compiler error: 'playername' is not a member of 'MyChunk'
```

## Benefits Summary

| Feature | Without Reflection | With Reflection |
|---------|-------------------|-----------------|
| Define fields | Once (struct) | Once (struct + descriptor) |
| GUI metadata | Manually in separate file | Automatic from struct |
| Field offsets | Calculate manually | `offsetof()` (automatic) |
| Field sizes | Calculate manually | `sizeof()` (automatic) |
| Type names | Type as string (manual) | `#Type` (automatic) |
| Maintainability | Update 2 places | Update 1 place |
| Compile-time safety | ❌ No checking | ✅ Compiler verifies |
| Sync errors | Easy to happen | Impossible |

## Next Steps

1. **Read the full guide**: [REFLECTION_SYSTEM.md](REFLECTION_SYSTEM.md)
2. **See working examples**: [examples/reflected_chunk_example.h](../examples/reflected_chunk_example.h)
3. **Start migrating**: Pick one simple struct and try it!

## Questions?

**Q: Do I have to migrate all structs at once?**
A: No! Old and new styles work side-by-side. Migrate gradually.

**Q: Does this add runtime overhead?**
A: No. All reflection happens at compile-time. Zero cost.

**Q: Will this work with existing code?**
A: Yes! It's fully backward compatible.

**Q: What about C++26 reflection?**
A: When available, we can migrate to native reflection easily. This is a stopgap solution.

## Comparison with Other Approaches

| Approach | Pros | Cons |
|----------|------|------|
| **Manual duplication** (current) | No dependencies | Error-prone, maintenance burden |
| **Python codegen** | Fully automatic | Build complexity, Python dependency |
| **Qt moc-style** | Industry standard | Requires custom tool, build complexity |
| **Macro reflection** (this solution) | ✅ No build changes<br>✅ Compile-time safe<br>✅ Easy to use | Requires macros (some boilerplate) |
| **C++26 native** | Perfect | Not available yet (2026+) |

## Success Story

After implementing reflection on `TechTreeUnitTypeNameChunk0x56`:

**Before:**
- Struct in `unit_chunks.h`: 5 lines
- GUI metadata in `chunk_metadata.cpp`: 10 lines
- Total: 15 lines, 2 files to maintain

**After:**
- Struct in `unit_chunks.h` with reflection: 8 lines (all in one place)
- GUI registration in `chunk_metadata.cpp`: 1 line
- Total: 9 lines, automatic sync

**Result:** 40% less code, 100% less sync errors! 🎉
