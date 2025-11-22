/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef TILE_H
#define TILE_H

// Standard libs
#include <optional>
#include <vector>
#include <unordered_map>

// JSON

// Application files
#include <defs/world_builder_defs.h>
#include <geo_models/tiles/coord.h>
#include <geo_models/tiles/terrain.h>

namespace world_builder
{

/**
 *  Forward declarations
 */
class Tiles_config;
class Tile;

/**
 * @brief World_tiles is the main container that will hold each tile. Keys are
 * hashed via `Coord_hash` to determine uniqueness in the map.
 */
using World_tiles = std::unordered_map<world_builder::Coord,
                                       world_builder::Tile,
                                       world_builder::Coord_hash>;

/**
 * @brief The Tile class
 */
class Tile
{
public:
  // Attributes

  // Implementation
  /**
   * @brief Constructor
   * @param coord
   */
  Tile(const Coord& coord);

  /**
   * @brief Copy constructor
   * @param copy
   */
  Tile(const Tile& copy);

  /**
   * @brief Assignment operator
   * @param other Other tile to assign
   * @return
   */
  Tile& operator=(const Tile& other)
  {
    if (this != &other)
    {
      // m_neighbor_offsets is const, so we can't reassign it
      m_coord = other.m_coord;
      m_elevation = other.m_elevation;
      m_terrain = other.m_terrain;
      m_is_river = other.m_is_river;
      m_is_coast = other.m_is_coast;
      m_river_to = other.m_river_to;
    }
    return *this;
  }

  /**
   * @brief Get neighbor tiles
   * @param coord The coordinate to get neighbors for
   * @return The coordinates of the neighboring tiles
   */
  std::vector<Coord> Get_neighbor_tiles(const Coord& coord);

  /**
   * @brief Search all neighbor tiles for a downhill neighbor
   * @param c
   * @param tiles
   * @return The lowest downhill neighboring tile
   */
  std::optional<world_builder::Coord> Downhill_neighbor(const world_builder::Coord& c,
                                                        const world_builder::World_tiles& tiles);

  /**
   * @brief Trace_river
   * @param start
   * @param tiles
   * @param P
   * @return
   */
  std::vector<Coord> Trace_river(const Coord& start,
                                 world_builder::World_tiles& tiles,
                                 const Tiles_config& P);

  /**
   * @brief Set ocean terrain for this tile, based on sea level
   * @param sea_level The configured sea level
   */
  void Set_ocean_terrain(const double sea_level);

  /**
   * @brief Paint terrain based on sea level
   * @param sea_level The configured sea level
   */
  void Paint_terrain(const double sea_level);

  /**
   * Getters and setters
   */
  const std::vector<Coord>& Get_neighbor_offsets() const { return m_neighbor_offsets; }

  const Coord& Get_coord() const { return m_coord; }

  const double Get_elevation() const { return m_elevation; }
  void Set_elevation(const double elevation) { m_elevation = elevation; }

  const world_builder::ETerrain Get_terrain() const { return m_terrain; }
  void Set_terrain(const world_builder::ETerrain& terrain) { m_terrain = terrain; }

  const bool Get_is_river() const { return m_is_river; }
  void Set_is_river(const bool river) { m_is_river = river; }

  const bool Get_is_coast() const { return m_is_coast; }
  void Set_is_coast(const bool coast) { m_is_coast = coast; }

  const std::optional<Coord>& Get_river_to() const { return m_river_to; }
  void Set_river_to(const Coord& river_to) { m_river_to = river_to;}


private:
  // Attributes

  // Implementation
  /**
   * @brief Axial hex neighbor offsets ie making hexes
   */
  static const std::vector<Coord> m_neighbor_offsets;

  /**
   * @brief The coordinates of this tile
   */
  Coord m_coord;

  /**
   * @brief Elevation of this tile
   */
  double m_elevation;

  /**
   * @brief Terrain type of this tile
   */
  world_builder::ETerrain m_terrain;

  /**
   * @brief This is a river tile
   */
  bool m_is_river;

  /**
   * @brief This is a coastal tile
   */
  bool m_is_coast;

  /**
   * @brief The downstream river tile, if this is a river tile
   */
  std::optional<Coord> m_river_to;
};
}

#endif
