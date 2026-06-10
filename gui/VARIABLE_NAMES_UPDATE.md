# Variable Names Update

## Summary

Updated the chunk metadata registry in [gui/chunk_metadata.cpp](gui/chunk_metadata.cpp) to display actual variable/field names for all chunk types instead of generic "byte[]" placeholders.

## What Was Changed

### Before
The properties panel would show:
- "No field metadata available" for most chunk types
- Generic descriptions like "byte[580]" or "byte[12][]"
- Only a few chunks (MAP_STRUCTURE, METADATA) had proper field definitions

### After
The properties panel now shows:
- **Actual variable names** from the struct definitions
- **Proper data types** (uint32_t, char16_t[], etc.)
- **Descriptive tooltips** explaining what each field contains
- **Field offsets** within the chunk data

## Chunks Updated

### Player Chunks
- **PLAYER_PROPERTIES** (236 bytes):
  - `player_index`, `nation_index`, `player_flags`
  - `player_name` (UTF-16), `control_field` (0=AI, 4=human)
  - `color_index`, `color_hex_1/2/3`, `difficulty`

- **PLAYER_DETAILS** (12 bytes each):
  - `player_index`, `player_data_value`, `flags`

### Formation/Unit Chunks
- **FORMATION_PROPERTIES** (580 bytes):
  - `player_index`, `formation_unit_id`, linked list pointers
  - `formation_type`, `formation_flags`, behavior flags
  - `formation_unit_name` (UTF-16), `position_x/y`
  - `damage_taken`, `group_id`, `unit_index_in_group`

- **BUILDING_GROUP_PROPERTIES** (548 bytes):
  - `player_index`, `entity_index`, `additional_data`
  - `encrypted_param_1/2` (XOR 0x63637 - health, attack, etc.)
  - `entity_name` (UTF-16), `capability_type_flag`

- **GARRISON_COUNT** (8 bytes):
  - `unit_count`, `garrison_type_flag`

- **GARRISON_UNIT_NAMES** (512 bytes each):
  - `unit_name` (UTF-16)

### Object Chunks
- **UNIT_OBJECT** (804 bytes):
  - `job_time`, `property_0x04`, `property_0x10`
  - `unit_name` (UTF-16), `string_data_2` (UTF-16)
  - Resource costs: `food_cost`, `timber_cost`, `wealth_cost`, `knowledge_cost`, `metal_cost`, `oil_cost`
  - Combat stats: `attack`, `min_range`, `max_range`, `hits`, `armor`, `line_of_sight`
  - Movement: `moves`, `turn_speed`, `craft`, `control_cost`

- **STRUCTURE_OBJECT** (816 bytes):
  - Similar to UNIT_OBJECT but with building-specific fields
  - Additional: `town_hits`, `plunder_amount`, `plunder_good`, `garrison_max`, `base_arrows`, `most_shots`, `wonder_points`

- **RESOURCE_OBJECT** (760 bytes):
  - `resource_name` (UTF-16), `string_data_2` (UTF-16)
  - Position: `resource_pos_x/y/z`
  - Properties: `resource_prop_18/1c/20/24/28/2c`

### Building Chunks
- **CITY_BUILDING_PROPERTIES** (752 bytes):
  - `player_index`, `building_index`, `additional_data`
  - `encrypted_param_1/2` (XOR 0x63637)
  - `city_flags`, `city_id`, `city_name` (UTF-16)
  - `building_name` (UTF-16), `city_capability_flag`

### Resource Chunks
- **RESOURCE_ENTRIES** (8 bytes each):
  - `resource_index` (0-5), `value`

### Trigger Chunks
- **TRIGGER_ENTRIES** (520 bytes each):
  - `trigger_name` (UTF-16)
  - `x_coordinate` (likely X coordinate for trigger-linked placements; previously `encrypted_param1`)
  - `y_coordinate` (likely Y coordinate for trigger-linked placements; previously `encrypted_param2`)

### Map Resource Chunks
- **MAP_RESOURCE_ENTRIES** (520 bytes each):
  - `name` (UTF-16LE map resource type name)
  - `position_x/y` (map resource coordinates)

### Advanced Feature Chunks
- **FEATURE_DATA** (12 bytes each) - Mountain features:
  - `feature_id`, `mountain_flag1/2`, `padding`, `feature_properties`

- **LOCATION_DATA** (12 bytes each) - Locations / terrain decorations:
  - `x_coordinate`, `y_coordinate`, `flags_and_id`

- **COORDINATE_DATA** (8 bytes each) - Spatial groups:
  - `x_processed`, `y_processed`

- **ENTITY_DATA** (16 bytes each):
  - `field_0x40/44/38/3c` (can be float or int)

### Diplomacy Chunks
- **DIPLOMATIC_ENTRIES** (520 bytes each):
  - `agreement_name` (UTF-16)
  - `diplomatic_terms` (treaty duration, trade values, military support)
  - `diplomatic_status` (alliance level, agreement state, relationship)

### Editor Chunks
- **EDITOR_MAP_TERRAIN_DATA** (520 bytes):
  - `group_config`, `units_in_group` (0-128)
  - `unit_ids` (array of 128 uint32_t)

- **EDITOR_RESEARCH_PROGRESS** (16 bytes each):
  - `element_config`, `element_type`
  - `x_coordinate`, `y_coordinate`

### Scenario Header
- **METADATA** (384 bytes) - ScenarioHeader0x6C:
  - `version`, `magic` (0xf00dcafe)
  - `scenario_name` (UTF-16)
  - Multiple player settings, global settings, game flags
  - `ai_difficulty`, `start_age`, `end_age`
  - 24 global setting values
  - `some_setting`, `Population`

## Technical Details

### Field Information Structure
Each field now includes:
- **Name**: Variable name from the C++ struct definition
- **Offset**: Byte offset within the chunk data (excluding chunk header)
- **Size**: Size in bytes
- **Type**: C++ type (uint32_t, char16_t[], etc.)
- **Description**: Human-readable explanation with additional context

### Encrypted Fields
Many chunks use XOR encryption (key 0x63637) for sensitive game data:
- Health, armor, production values
- Attack, range, capacity values
- Trigger conditions and actions
- Economic parameters for map location???

The GUI displays these as hex values - decryption would be a future enhancement.

### UTF-16 String Fields
All player-visible text uses UTF-16LE encoding:
- Player names
- Unit/building names
- City names
- Scenario names
- Trigger descriptions
- Diplomatic agreement names

The properties panel correctly decodes and displays these strings.

### Variable-Length Chunks
For chunks with repeating structures (e.g., PLAYER_DETAILS has multiple 12-byte PlayerEntry structs):
- The field definitions describe a SINGLE entry structure
- The properties panel now displays **ALL elements** in the array
- Each element is prefixed with an index: `[0]`, `[1]`, `[2]`, etc.
- The chunk header shows the total element count

**How it works:**
- If `chunk.data.size > chunkInfo.dataSize`, the GUI automatically detects multiple elements
- Element count = `chunk.data.size / chunkInfo.dataSize`
- All fields are repeated for each element with proper offset calculation
- Field names show as `[0] unit_type_name`, `[1] unit_type_name`, etc.

Supported variable-length chunks include:
- **TECH_TREE_UNIT_TYPE_NAMES** (512 bytes each): `name` (UTF-16LE nation Library research name)
- **TECH_TREE_BUILDING_TYPE_NAMES** (512 bytes each): `building_type_name` (UTF-16)
- **GARRISON_UNIT_NAMES** (512 bytes each): `unit_name` (UTF-16)
- **PLAYER_DETAILS** (12 bytes each): `player_index`, `player_data_value`, `flags`
- **RESOURCE_ENTRIES** (8 bytes each): `resource_index`, `value`
- **FEATURE_DATA** (12 bytes each): mountain/terrain features
- **LOCATION_DATA** (12 bytes each): location / terrain decoration coordinates
- **COORDINATE_DATA** (8 bytes each): spatial group coordinates
- **ENTITY_DATA** (16 bytes each): advanced feature entities
- And many more...

## How to Use

1. **Open a .scx file** in the GUI
2. **Click on any chunk** in the tree view on the left
3. **View the Properties panel** on the right to see:
   - Field Name (e.g., "player_name")
   - Type (e.g., "char16_t[100]")
   - Offset (e.g., "20")
   - Value (decoded - e.g., "Player 1" or "Red")
4. **Hover over values** to see tooltip descriptions

## Examples

### Example 1: Player Properties
When you click on a PLAYER_PROPERTIES chunk, you'll now see:
```
Field Name         | Type        | Offset | Value
---------------------------------------------------
player_index       | uint32_t    | 0      | 0
nation_index       | uint32_t    | 4      | 12
player_flags       | uint32_t    | 8      | 0x00000001
field_0x150        | int32_t     | 12     | -1
field_0x14c        | uint32_t    | 16     | 0
player_name        | char16_t[100]| 20    | Player 1
control_field      | uint32_t    | 220    | 4
remaining_field2   | uint32_t    | 224    | 0
color_index        | uint8_t     | 228    | 0
color_hex_1        | uint8_t     | 229    | 255
color_hex_2        | uint8_t     | 230    | 0
color_hex_3        | uint8_t     | 231    | 0
difficulty         | uint32_t    | 232    | 2
```

### Example 2: Formation Properties
When you click on a FORMATION_PROPERTIES chunk, you'll see:
```
Field Name             | Type            | Offset | Value
------------------------------------------------------------
player_index           | uint32_t        | 0      | 0
formation_unit_id      | uint32_t        | 4      | 12345
ll_prev_group_member   | int32_t         | 8      | -1
ll_next_group_member   | int32_t         | 12     | -1
formation_flags        | uint32_t        | 28     | 0x00000001
formation_type         | uint16_t        | 32     | 5
formation_unit_name    | char16_t[257]   | 38     | Infantry
position_x             | uint32_t        | 552    | 100
position_y             | uint32_t        | 556    | 150
damage_taken           | uint32_t        | 560    | 25
group_id               | uint32_t        | 572    | 3
unit_index_in_group    | uint32_t        | 576    | 0
```

### Example 3: Unit Object
When you click on a UNIT_OBJECT chunk, you'll see all the unit stats:
```
Field Name        | Type            | Offset | Value
-------------------------------------------------------
unit_name         | char16_t[256]   | 12     | Light Infantry
food_cost         | uint32_t        | 736    | 20
timber_cost       | uint32_t        | 740    | 10
attack            | uint32_t        | 760    | 8
min_range         | uint32_t        | 764    | 0
max_range         | uint32_t        | 768    | 1
hits              | uint32_t        | 772    | 80
armor             | uint32_t        | 776    | 2
line_of_sight     | uint32_t        | 780    | 5
moves             | uint32_t        | 788    | 3
```

### Example 4: Tech Tree Unit Type Names (Variable-Length Array)
When you click on a TECH_TREE_UNIT_TYPE_NAMES chunk with 3 unit types, you'll see:
```
Chunk Header:
  Array Elements: 3 (each 512 bytes)

Field Name               | Type            | Offset | Value
-------------------------------------------------------------
[0] unit_type_name       | char16_t[256]   | 0      | Light Infantry
[1] unit_type_name       | char16_t[256]   | 512    | Heavy Infantry
[2] unit_type_name       | char16_t[256]   | 1024   | Archer
```

This automatically displays all 3 elements with:
- Element index prefix `[0]`, `[1]`, `[2]`
- Proper offset calculation for each element
- Decoded UTF-16 strings for each unit type name

## Benefits

1. **Easier Debugging**: See exactly what each byte represents
2. **Better Understanding**: Learn the .scx file format structure
3. **Scenario Editing**: Know which fields to modify for specific changes
4. **Reverse Engineering**: Identify unknown fields and their purposes
5. **Documentation**: Field descriptions serve as inline documentation

## Recent Enhancements

Implemented features:
1. ✅ **Array Support**: Now displays all entries for variable-length chunks with element indexing
   - Automatically detects multiple elements based on chunk size
   - Shows `[0]`, `[1]`, `[2]` prefixes for each array element
   - Displays total element count in chunk header

## Future Enhancements

Potential improvements:
1. **XOR Decryption**: Automatically decrypt encrypted fields (key 0x63637)
2. **Enum Display**: Show enum names instead of numeric values (e.g., "Red" instead of 0 for colors)
3. **Field Editing**: Allow editing values directly in the properties panel
4. **Calculated Fields**: Show derived values (e.g., current_health = max_health - damage_taken)
5. **Hex Editor Sync**: Clicking a field highlights the corresponding bytes in the hex view
6. **Array Element Grouping**: Add visual separators or collapsible sections between array elements

## Files Modified

- `gui/chunk_metadata.cpp` - Added detailed field definitions for 30+ chunk types, including tech tree name chunks
- `gui/chunk_properties_widget.cpp` - Implemented automatic array element iteration and display
- `gui/VARIABLE_NAMES_UPDATE.md` - Updated documentation to reflect array support
- All changes are backward compatible

## Build Status

✅ Successfully compiled with no errors or warnings
✅ Tested with Qt6 6.10.1 MinGW 64-bit
✅ All existing functionality preserved
✅ No breaking changes to the GUI architecture

## Testing

To test the updated variable names:
1. Run the GUI: `run-gui.bat` or `./build-mingw/gui/scenario_viewer_gui.exe`
2. Open a .scx file (e.g., `9.scx` or `rio.scx`)
3. Click through different chunk types in the tree view
4. Verify the Properties panel shows meaningful field names and values
5. Check that UTF-16 strings display correctly
6. Hover over values to see tooltip descriptions

Expected results:
- Most chunks now show 5-30 individual fields with proper names
- No more "No field metadata available" for major chunk types
- Player names, unit names, and scenario names display as readable text
- Numeric fields show decoded integer values
- Field tooltips provide helpful context

## References

All field definitions sourced from:
- `include/chunks/player_chunks.h`
- `include/chunks/unit_chunks.h`
- `include/chunks/building_chunks.h`
- `include/chunks/object_chunks.h`
- `include/chunks/map_chunks.h`
- `include/chunks/scenario_chunks.h`
- `include/chunks/resource_chunks.h`
- `include/chunks/advanced_feature_chunks.h`
- `include/chunks/diplomatic_chunks.h`
- `include/chunks/editor_chunks.h`
- `include/chunks/visibility_chunks.h`

## Notes

- Container chunks (headers with no data) still show as containers
- Some chunks have unknown/placeholder field names (e.g., `field_0x10`) where the purpose isn't documented
- Encrypted fields are shown as-is (not decrypted) - decryption would require additional implementation
- Variable-length array chunks show the structure of a single element
