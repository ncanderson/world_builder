/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef WORLD_H
#define WORLD_H

// Standard libs

// JSON

// Application files
#include <defs/world_builder_defs.h>
#include <geo_models/tiles/tile.h>

namespace world_builder
{

class Continent;
class Tiles_config;

/**
 * @brief The World class
 */
class World
{
public:
  // Attributes

  // Implementation
  /**
   * @brief Constructor
   * @param params
   */
  World(const Tiles_config& tiles_config);

  /**
   * @brief Seed_continents
   */
  void Seed_continents();

  /**
   * @brief Seed_oceans
   */
  void Seed_oceans();

  /**
   * @brief Run_diffusion
   */
  void Run_diffusion();

  /**
   * @brief Normalize_elevation
   */
  void Normalize_elevation();

  /**
   * @brief Check the tiles elevation and specify Ocean and Coastal terrain
   */
  void Run_oceans_and_coasts();

  /**
   * @brief Run_rivers
   */
  void Run_rivers();

  /**
   * @brief Paint the terrain on each tile
   */
  void Paint_terrain();

  /**
   * Getters and setters
   */
  const world_builder::World_tiles Get_world_tiles() const { return m_world_tiles; }
  const std::vector<std::vector<world_builder::Coord>> Get_rivers() const { return m_rivers; }

private:
  // Attributes
  /**
   * @brief Params
   */
  const Tiles_config& m_tiles_config;

  /**
   * @brief The tiles making up the world
   */
  world_builder::World_tiles m_world_tiles;

  /**
   * @brief continents
   */
  std::vector<world_builder::Continent> m_continents;

  /**
   * @brief Max number of elevation seeds per continent
   */
  uint8_t m_seeds_per_continent;

  /**
   * @brief List of all rivers
   */
  std::vector<std::vector<world_builder::Coord>> m_rivers;

  // Implementation
};
}

#endif
