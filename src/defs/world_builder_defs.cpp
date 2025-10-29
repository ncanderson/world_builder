/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <functional>

// JSON

// Application files
#include <defs/world_builder_defs.h>
#include <geo_models/coord.h>

///////////////////////////////////////////////////////////////////////

std::size_t world_builder::Coord_hash::operator()(const Coord& coord) const noexcept
{
  return (std::hash<int>()(coord.Get_q_coord()) << 1) ^ std::hash<int>()(coord.Get_r_coord());
}

///////////////////////////////////////////////////////////////////////
