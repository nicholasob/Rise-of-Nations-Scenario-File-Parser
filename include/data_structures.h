#pragma once

//base classes and utilities
#include "base/byte_convertible.h"
#include "base/chunk_base.h"
#include "common/common_types.h"

//chunk categories
#include "chunks/player_chunks.h"
#include "chunks/map_chunks.h"
#include "chunks/resource_chunks.h"
#include "chunks/unit_chunks.h"
#include "chunks/building_chunks.h"
#include "chunks/editor_chunks.h"
#include "chunks/scenario_chunks.h"
#include "chunks/object_chunks.h"
#include "chunks/visibility_chunks.h"
#include "chunks/advanced_feature_chunks.h"
#include "chunks/diplomatic_chunks.h"

//include external dependencies
#include "chunk_types.h"

//standard library includes
#include <vector>
#include <cstring>
#include <string>
#include <cstdint>
#include <stdexcept>
