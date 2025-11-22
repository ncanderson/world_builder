/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <algorithm>

// JSON

// Application files
#include <geo_models/tiles/tile.h>
#include <utils/tiles_config.h>

///////////////////////////////////////////////////////////////////////

using tile = world_builder::Tile;
const std::vector<world_builder::Coord> tile::Tile::m_neighbor_offsets = {
    {1, 0}, {1, -1}, {0, -1}, {-1, 0}, {-1, 1}, {0, 1}
};

///////////////////////////////////////////////////////////////////////

tile::Tile(const world_builder::Coord& coord)
  :
  m_coord(coord),
  m_elevation(0.0),
  m_terrain(world_builder::ETerrain::ETERRAIN_Unknown),
  m_is_river(false),
  m_is_coast(false),
  m_river_to()
{ }

///////////////////////////////////////////////////////////////////////

tile::Tile(const Tile& copy)
  :
  m_coord(copy.m_coord),
  m_elevation(copy.m_elevation),
  m_terrain(copy.m_terrain),
  m_is_river(copy.m_is_river),
  m_is_coast(copy.m_is_coast),
  m_river_to(copy.m_river_to)
{ }

///////////////////////////////////////////////////////////////////////

std::vector<world_builder::Coord> tile::Get_neighbor_tiles(const world_builder::Coord& coord)
{
  std::vector<Coord> neighbors;
  for (auto& offset_coord :   m_neighbor_offsets)
  {
    neighbors.push_back({coord.Get_q_coord() + offset_coord.Get_q_coord(),
                         coord.Get_r_coord() + offset_coord.Get_r_coord()});
  }
  return neighbors;
}

///////////////////////////////////////////////////////////////////////

std::optional<world_builder::Coord> tile::Downhill_neighbor(const world_builder::Coord& c,
                                                            const world_builder::World_tiles& tiles)
{
  auto it = tiles.find(c);
  if(it == tiles.end())
  {
    return std::nullopt;
  }

  double cur_e = it->second.m_elevation;

  std::vector<std::pair<world_builder::Coord, double>> cands;

  for(auto& n : Get_neighbor_tiles(c))
  {
    auto it2 = tiles.find(n);
    if(it2 != tiles.end())
    {
      cands.push_back({n, it2->second.m_elevation});
    }
  }

  if(cands.empty())
  {
    return std::nullopt;
  }

  std::sort(cands.begin(),
            cands.end(),
            [](auto& a, auto& b){ return a.second < b.second; });
  if(cands.front().second <= cur_e)
  {
    return cands.front().first;
  }
  return std::nullopt;
}

///////////////////////////////////////////////////////////////////////

std::vector<world_builder::Coord> tile::Trace_river(const Coord& start,
                                                    world_builder::World_tiles& tiles,
                                                    const Tiles_config& params)
{
  std::vector<Coord> path;
  Coord cur = start;
  std::unordered_map<Coord, bool, Coord_hash> visited;
  for(int step = 0; step < params.Get_max_river_length(); ++step)
  {
    if(visited[cur])
    {
      break;
    }
    visited[cur] = true;
    path.push_back(cur);

    auto current_tile = tiles.find(cur);
    if(current_tile != tiles.end())
    {
      if(current_tile->second.Get_elevation() <= params.Get_sea_level())
      {
        break;
      }
    }

    auto dn = Downhill_neighbor(cur, tiles);
    if(!dn)
    {
      break;
    }
    cur = *dn;
  }
  return path;
}

///////////////////////////////////////////////////////////////////////

void tile::Set_ocean_terrain(const double sea_level)
{
  if(m_elevation <= sea_level)
  {
    m_terrain = ETerrain::ETERRAIN_Ocean;
  }
}

///////////////////////////////////////////////////////////////////////

void tile::Paint_terrain(const double sea_level)
{
  if (m_terrain == world_builder::ETerrain::ETERRAIN_Ocean)
  {
    return;
  }

  if(m_is_river)
  {
    m_terrain = world_builder::ETerrain::ETERRAIN_River;
  }

  if(m_is_coast && m_elevation <= sea_level + 0.03)
  {
    m_terrain = world_builder::ETerrain::ETERRAIN_Beach;
  }
  else if(m_elevation < sea_level + 0.07)
  {
    m_terrain = world_builder::ETerrain::ETERRAIN_Marsh;
  }
  else if(m_elevation < sea_level + 0.20)
  {
    m_terrain = world_builder::ETerrain::ETERRAIN_Plains;
  }
  else if(m_elevation < sea_level + 0.45)
  {
    m_terrain = world_builder::ETerrain::ETERRAIN_Hills;
  }
  else
  {
    if(m_elevation > 0.8)
    {
      m_terrain = world_builder::ETerrain::ETERRAIN_Mountains;
    }
    else
    {
      m_terrain = world_builder::ETerrain::ETERRAIN_Hills;
    }
  }
}

///////////////////////////////////////////////////////////////////////
