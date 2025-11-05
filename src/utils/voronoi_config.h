/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef VORONOI_CONFIG_H
#define VORONOI_CONFIG_H

// Standard libs
#include <fstream>
#include <cstdint>
#include <string>

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
   * @brief Getters
   */
  const unsigned Get_seed() const { return m_seed; }
  const double Get_resolution() const { return m_resolution; }
  const std::string& Get_outfile_ext() const { return m_outfile_ext; }
  const std::string& Get_outfile() const { return m_outfile; }
  const double Get_erosion_amount() const { return m_erosion_amount; }
  const uint8_t Get_erosion_iterations() const { return m_erosion_iterations; }
  const uint8_t Get_num_cities() const { return m_num_cities; }
  const uint8_t Get_num_towns() const { return m_num_towns; }
  const uint32_t Get_image_width() const { return m_image_width; }
  const uint32_t Get_image_height() const { return m_image_height; }
  const double Get_default_extents_height() const { return m_default_extents_height; }
  const double Get_draw_scale() const { return m_draw_scale; }

  const bool Get_enable_slopes() const { return m_enable_slopes; }
  const bool Get_enable_rivers() const { return m_enable_rivers; }
  const bool Get_enable_contour() const { return m_enable_contour; }
  const bool Get_enable_borders() const { return m_enable_borders; }
  const bool Get_enable_cities() const { return m_enable_cities; }
  const bool Get_enable_towns() const { return m_enable_towns; }
  const bool Get_enable_labels() const { return m_enable_labels; }
  const bool Get_enable_area_labels() const { return m_enable_area_labels; }

  const bool Get_verbose() const { return m_verbose; }

private:
  // Attributes
  /**
   * @brief Random seed for map generation
   */
  unsigned m_seed;

  /**
   * @brief Controls granularity of Voronoi cells
   */
  double m_resolution;

  /**
   * @brief Output file extension (e.g. .png or .json)
   */
  std::string m_outfile_ext;

  /**
   * @brief Output file name
   */
  std::string m_outfile;

  /**
   * @brief Erosion strength (negative means auto)
   */
  double m_erosion_amount;

  /**
   * @brief Number of erosion passes
   */
  uint8_t m_erosion_iterations;

  /**
   * @brief Number of generated cities
   */
  uint8_t m_num_cities;

  /**
   * @brief Number of generated towns
   */
  uint8_t m_num_towns;

  /**
   * @brief Image output width in pixels
   */
  uint32_t m_image_width;

  /**
   * @brief Image output height in pixels
   */
  uint32_t m_image_height;

  /**
   * @brief Base map vertical scale
   */
  double m_default_extents_height;

  /**
   * @brief Drawing scale factor
   */
  double m_draw_scale;

  /**
   * @brief Enable slope generation
   */
  bool m_enable_slopes;

  /**
   * @brief Enable river generation
   */
  bool m_enable_rivers;

  /**
   * @brief Enable contour drawing
   */
  bool m_enable_contour;

  /**
   * @brief Enable political borders
   */
  bool m_enable_borders;

  /**
   * @brief Enable city generation
   */
  bool m_enable_cities;

  /**
   * @brief Enable town generation
   */
  bool m_enable_towns;

  /**
   * @brief Enable text labels
   */
  bool m_enable_labels;

  /**
   * @brief Enable area labels
   */
  bool m_enable_area_labels;

  /**
   * @brief Verbose logging
   */
  bool m_verbose;

  // Implementation

};
}

#endif
