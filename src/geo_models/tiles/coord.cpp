/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs

// JSON

// Application files
#include <geo_models/tiles/coord.h>
#include <defs/world_builder_defs.h>

///////////////////////////////////////////////////////////////////////

using coord = world_builder::Coord;

///////////////////////////////////////////////////////////////////////

coord::Coord(int32_t q, int32_t r)
  :
  m_q_coord(q),
  m_r_coord(r)
{ }

///////////////////////////////////////////////////////////////////////

bool coord::operator==(const Coord& other) const noexcept
{
  return (m_q_coord == other.m_q_coord) && (m_r_coord == other.m_r_coord);
}

///////////////////////////////////////////////////////////////////////
