/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs

// JSON

// Application files
#include <defs/dice_rolls.h>
#include <geo_models/tiles/continent.h>
#include <geo_models/tiles/world.h>
#include <utils/tiles_config.h>

///////////////////////////////////////////////////////////////////////

using wd = world_builder::World;

///////////////////////////////////////////////////////////////////////

wd::World(const Tiles_config& tiles_config)
  :
  m_tiles_config(tiles_config),
  m_world_tiles(),
  m_continents(),
  m_seeds_per_continent(0)
{
  // Using the params, build a grid of Coord objects, which are then used to
  // build a Tile. These tiles represent the individual unit that builds the
  // whole map.
  for(int q = 0; q < tiles_config.Get_width(); ++q)
  {
    for(int r = 0; r < tiles_config.Get_height(); ++r)
    {
      world_builder::Coord new_coord = world_builder::Coord(q, r);
      m_world_tiles.try_emplace(new_coord, world_builder::Tile(new_coord));
    }
  }
}

///////////////////////////////////////////////////////////////////////

void wd::Seed_continents()
{
  // Define 2–4 continents depending on map size
  int num_continents = std::max(static_cast<uint32_t>(2),
                                m_tiles_config.Get_width() / 40);

  // Padding around the edge of the map, so continents don't wrap around the edge
  for (int i = 0; i < num_continents; ++i)
  {
    m_continents.push_back({
        world_builder::dice::Make_a_roll<int32_t>(m_tiles_config.Get_width() / 8, m_tiles_config.Get_width() * 7 / 8),
        world_builder::dice::Make_a_roll<int32_t>(m_tiles_config.Get_height() / 8, m_tiles_config.Get_height() * 7 / 8),
        world_builder::dice::Make_a_roll<double>(m_tiles_config.Get_width() / 6.0, m_tiles_config.Get_width() / 4.0)
    });
  }

  // Place elevation seeds around each continent center
  m_seeds_per_continent = std::max(static_cast<uint32_t>(3),
                                   (m_tiles_config.Get_width() * m_tiles_config.Get_height()) / (200 * num_continents));
  for (auto& c : m_continents)
  {
    for (int i = 0; i < m_seeds_per_continent; ++i)
    {
      double angle = world_builder::dice::Make_a_roll<double>(0, 2 * M_PI);
      double dist = world_builder::dice::Make_a_roll<double>(0, c.Get_radius());
      int q_coord = c.Get_center_q() + static_cast<int>(dist * cos(angle));
      int r_coord = c.Get_center_r() + static_cast<int>(dist * sin(angle));

      if(q_coord >= 0 && q_coord < m_tiles_config.Get_width() && r_coord >= 0 && r_coord < m_tiles_config.Get_height())
      {
        // Skewed toward land
        world_builder::Coord coord = world_builder::Coord(q_coord, r_coord);
        auto it = m_world_tiles.find(coord);
        if (it != m_world_tiles.end())
        {
          it->second.Set_elevation(world_builder::dice::Make_a_roll<double>(0.4, 1.0));
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////

void wd::Seed_oceans()
{
  // Only add ocean seeds outside continents
  int oceanSeeds = m_seeds_per_continent; // same count as land seeds
  for (int i = 0; i < oceanSeeds; ++i)
  {
    int q = world_builder::dice::Make_a_roll<int>(0, m_tiles_config.Get_width() - 1);
    int r = world_builder::dice::Make_a_roll<int>(0, m_tiles_config.Get_height() - 1);

    // skip tiles that are close to a continent center
    bool nearContinent = false;
    for (auto& c : m_continents)
    {
      double dq = q - c.Get_center_q();
      double dr = r - c.Get_center_r();
      double dist = std::sqrt(dq*dq + dr*dr);
      if (dist < c.Get_radius() * 0.8)
      {
        nearContinent = true;
        break;
      }
    }

    if(!nearContinent)
    {
      // Skewed toward land
      world_builder::Coord coord = world_builder::Coord(q, r);
      auto it = m_world_tiles.find(coord);
      if (it != m_world_tiles.end())
      {
        it->second.Set_elevation(world_builder::dice::Make_a_roll<double>(-0.5, 0.2));
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////

void wd::Run_diffusion()
{
  // Diffusion / smoothing. For every smoothing pass, this will set the elevation for each
  // to based on the average of all neighbors with some random noise injected.
  for (int pass = 0; pass < m_tiles_config.Get_smooth_passes(); ++pass)
  {
    // Temp map to hold all coordinates, allowing update of the main map after
    // doing the smoothing passes
    std::unordered_map<world_builder::Coord,
                       double,
                       world_builder::Coord_hash> new_elev;

    // ie for every tile...
    for (auto& [coord, tile] : m_world_tiles)
    {
      // get all neighbors for this tile
      auto neighbors = tile.Get_neighbor_tiles(coord);

      // Neighboring elevations
      std::vector<double> neighbor_elevations;

      // for all neighbors,
      for (auto& nn : neighbors)
      {
        // Get the pointed to neighbor tile
        auto it2 = m_world_tiles.find(nn);
        if(it2 != m_world_tiles.end())
        {
          // If found, add the elevation
          neighbor_elevations.push_back(it2->second.Get_elevation());
        }
      }

      // Average all neighboring elevations
      double nbr_mean = 0;

      // If no neighbors, use this tile's own elevation
      if(neighbor_elevations.empty())
      {
        nbr_mean = tile.Get_elevation();
      }
      // Otherwise, add up all neighboring elevations and divide by found neighbors
      else
      {
        nbr_mean = std::accumulate(neighbor_elevations.begin(),
                                   neighbor_elevations.end(), 0.0) / neighbor_elevations.size();
      }

      double blend = 0.6;

      // (rng.uniform() - 0.5): Make the number in the range of -.5 to .5
      // * params.randomness: Augment the random roll with the additional randomness factor
      // (1.0 - (double)pass / params.smooth_passes): Dampens the noise gradually with each smoothing pass.
      double noise = (world_builder::dice::Make_a_roll<double>(0, 1) - 0.5) * m_tiles_config.Get_randomness() * (1.0 - (double)pass / m_tiles_config.Get_smooth_passes());

      // New elevation is a weighted average between (old elevation) and (neighbor mean), plus some fading noise.
      // If blend = 0.5 → half current height, half neighbors → moderate smoothing.
      // If blend = 1.0 → completely replace with neighbor mean (max smoothing).
      // If blend = 0.0 → do nothing (preserve current map).
      new_elev[coord] = nbr_mean * blend + tile.Get_elevation() * (1 - blend) + noise;
    }

    // Reset the tiles to the new elevation
    for (const auto& [coord, val] : new_elev)
    {
      auto it = m_world_tiles.find(coord);
      if (it != m_world_tiles.end())
      {
        it->second.Set_elevation(val);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////

void wd::Normalize_elevation()
{
  // Normalize elevation
  double minE = 1e9;
  double maxE = -1e9;

  // For every tile, check for a new max or min elevation
  for(auto& [c, t] : m_world_tiles)
  {
    minE = std::min(minE, t.Get_elevation());
    maxE = std::max(maxE, t.Get_elevation());
  }

  // For every tile, re-scale the elevation to fit within a 0 - 1 range
  for(auto& [c, t] : m_world_tiles)
  {
    t.Set_elevation((t.Get_elevation() - minE) / (maxE - minE));
  }
}

///////////////////////////////////////////////////////////////////////

void wd::Run_oceans_and_coasts()
{
  // Ocean terrain classification based on elevation
  // This has to be done first, since the coastal checks need to know if any
  // neighbors are oceans
  for(auto& [c, t] : m_world_tiles)
  {
    t.Set_ocean_terrain(m_tiles_config.Get_sea_level());
  }

  // Mark coasts
  for(auto& [c, t] : m_world_tiles)
  {
    // Ignore oceans
    if(t.Get_terrain() == world_builder::ETerrain::ETERRAIN_Ocean)
    {
      continue;
    }

    // For every tile, if it's not an ocean but a neighbor is an ocean, then
    // this is a coast
    for(auto& n : t.Get_neighbor_tiles(c))
    {
      auto it2 = m_world_tiles.find(n);
      if(it2 != m_world_tiles.end() && it2->second.Get_terrain() == world_builder::ETerrain::ETERRAIN_Ocean)
      {
        t.Set_is_coast(true);
        break;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////

void wd::Run_rivers()
{
  // for every tile,
  for(auto& [c, t] : m_world_tiles)
  {
    // Check elevation. greater than sea level (plus a pad), and make a roll against probability
    if(t.Get_elevation() > m_tiles_config.Get_sea_level() + 0.05 && world_builder::dice::Make_a_roll<double>(0, 1) < m_tiles_config.Get_river_spawn_prob())
    {
      // Trace a river path,
      auto path = t.Trace_river(c, m_world_tiles, m_tiles_config);
      // if there are three or more tiles,
      if (path.size() >= 3)
      {
        // for each tile in the river path,
        for(size_t i = 0; i + 1 < path.size(); ++i)
        {
          auto it = m_world_tiles.find(path[i]);
          if (it != m_world_tiles.end())
          {
            it->second.Set_is_river(true);
            it->second.Set_river_to(path[i + 1]);
          }
        }

        // handle the last tile
        auto it_last = m_world_tiles.find(path.back());
        if(it_last != m_world_tiles.end())
        {
          it_last->second.Set_is_river(true);
        }

        m_rivers.push_back(path);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////

void wd::Paint_terrain()
{
  for (auto& [coord, tile] : m_world_tiles)
  {
    tile.Paint_terrain(m_tiles_config.Get_sea_level());
  }
}

///////////////////////////////////////////////////////////////////////
