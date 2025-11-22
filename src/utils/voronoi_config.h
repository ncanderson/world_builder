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
  const double Get_voronoi_scale_factor() const { return m_voronoi_scale_factor; }

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

  /**
   * @brief Scale factor for the Voronoi builder.
   * @details Smaller scale factor means coarser rounding, some points may collapse
   * if decimal precision is removed, and Voronoi cells can become slightly more uniform
   * with less variation in shape and size.
   * A larger scale factor means more precise site positions, cells preserve
   * subtle differences, and slightly more irregular/organic-looking cells.
   */
  double m_voronoi_scale_factor;

  // Implementation

};
}

#endif
