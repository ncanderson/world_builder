/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef VORONOI_CONFIG_H
#define VORONOI_CONFIG_H

// Standard libs
#include <fstream>

// JSON

// Application files

namespace world_builder
{
/**
 * @brief Config for the Voronoi-based map generation algorithm
 */
class Voronoi_config
{
public:
  // Implementation
  /**
   * @brief Constructor
   * @param params_path Path to the JSON configuration file
   */
  Voronoi_config(std::ifstream params_path);

  /**
   * @brief Default ctor
   */
  Voronoi_config();

  /**
   * Getters
   */
  const double Get_width() const { return m_width; }
  const double Get_height() const { return m_height; }
  const double Get_min_distance() const { return m_min_distance; }
  const int Get_attempts() const { return m_k_attempts; }

private:
  // Attributes
  /**
   * @brief Width of the map
   */
  double m_width;

  /**
   * @brief Height of the map
   */
  double m_height;

  /**
   * @brief Min point distance
   */
  double m_min_distance;

  /**
   * @brief Number of attempts to find a new valid point
   */
  int m_k_attempts;

  // Implementation

};
}

#endif
