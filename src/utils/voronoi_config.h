/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef PARAMS_H
#define PARAMS_H

// Standard libs
#include <fstream>
#include <cstdint>

// JSON

// Application files

namespace world_builder
{
/**
 * @brief The Params class
 */
class Params
{
public:
  // Attributes

  // Implementation
  /**
   * @brief Constructor
   * @param Path to the params file
   */
  Params(std::ifstream params_path);

  /**
   * @brief Default ctor, to support conditional instantiation in main()
   * @details The default constructor will set up the random seed, but nothing
   * else.
   */
  Params();

  /**
   * Getters
   */
  const uint32_t Get_width() const { return m_width; }
  const uint32_t Get_height() const { return m_height; }
  const uint8_t Get_smooth_passes() const { return m_smooth_passes; }
  const double Get_randomness() const { return m_randomness; }
  const double Get_sea_level() const { return m_sea_level; }
  const double Get_river_spawn_prob() const { return m_river_spawn_prob; }
  const uint32_t Get_max_river_length() const { return m_max_river_length; }
  const unsigned Get_seed() const { return m_seed; }

private:
  // Attributes
  /**
   * @brief Map width
   */
  uint32_t m_width;

  /**
   * @brief Map height
   */
  uint32_t m_height;

  /**
   * @brief Fewer passes give a rougher map
   */
  uint8_t m_smooth_passes;

  /**
   * @brief Global terrain roughness factor
   * @details Higher values yield more random terrain
   */
  double m_randomness;

  /**
   * @brief sea_level
   */
  double m_sea_level;

  /**
   * @brief river_spawn_prob
   */
  double m_river_spawn_prob;

  /**
   * @brief max_river_length
   */
  uint32_t m_max_river_length;

  /**
   * @brief Random seed, used to generate the rest of the randomness
   */
  unsigned m_seed;

  // Implementation
};
}

#endif
