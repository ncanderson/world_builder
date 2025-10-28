/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <functional>

// JSON

// Application files
#include <geo_models/coord.h>

///////////////////////////////////////////////////////////////////////

using coord = world_builder::Coord;
using coord_hash = world_builder::Coord_hash;

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

std::size_t coord_hash::operator()(const Coord& coord) const noexcept
{
  return (std::hash<int>()(coord.Get_q_coord()) << 1) ^ std::hash<int>()(coord.Get_r_coord());
}

///////////////////////////////////////////////////////////////////////
