/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef TERRAIN_H
#define TERRAIN_H

// Standard libs
#include <cstdint>
#include <string>
#include <array>

// JSON

// Application files
#include <utils/world_builder_utils.h>

namespace world_builder
{
/**
 * @brief Terrain types
 */
enum class ETerrain : uint8_t
{
  ETERRAIN_Unknown,
  ETERRAIN_Ocean,
  ETERRAIN_River,
  ETERRAIN_Beach,
  ETERRAIN_Marsh,
  ETERRAIN_Plains,
  ETERRAIN_Hills,
  ETERRAIN_Mountains,
  ETERRAIN_Count
};

/**
 * @brief Lookup table mapping all enumerated terrain types to their appropriate
 * string representations.
 */
constexpr std::array<Enum_mapping<ETerrain>,
                     static_cast<size_t>(ETerrain::ETERRAIN_Count)> TERRAIN_LOOKUP = {
  Enum_mapping{ETerrain::ETERRAIN_Unknown,   "Unknown"},
  Enum_mapping{ETerrain::ETERRAIN_Ocean,     "Ocean"},
  Enum_mapping{ETerrain::ETERRAIN_River,     "River"},
  Enum_mapping{ETerrain::ETERRAIN_Beach,     "Beach"},
  Enum_mapping{ETerrain::ETERRAIN_Marsh,     "Marsh"},
  Enum_mapping{ETerrain::ETERRAIN_Plains,    "Plains"},
  Enum_mapping{ETerrain::ETERRAIN_Hills,     "Hills"},
  Enum_mapping{ETerrain::ETERRAIN_Mountains, "Mountains"}
};

/**
 * @brief The terrain class
 */
class Terrain
{
public:
  // Attributes

  // Implementation

private:
  // Attributes

  // Implementation

};
}

#endif
