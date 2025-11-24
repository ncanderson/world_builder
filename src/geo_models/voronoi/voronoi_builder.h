/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef VORONOI_BUILDER_H
#define VORONOI_BUILDER_H

// Standard libs
#include <vector>

// Boost Polygon
#include <boost/polygon/voronoi.hpp>
#include <boost/polygon/point_data.hpp>

// Application files
#include <defs/dice_rolls.h>
#include <geo_models/voronoi/poisson_disc.h>

namespace world_builder
{

using namespace boost::polygon;

/**
 * @brief Voronoi cell representation
 */
struct Cell
{

  /**
   * @brief ID of the cell, used to preserve color for the output
   */
  int id;

  /**
   * @brief A point in the Poisson grid
   */
  Point site;

  /**
   * @brief The vertices of the point
   */
  std::vector<Point> vertices;

  /**
   * @brief The random color for visualization
   */
  std::array<unsigned char, 3> color;
};

/**
 * @brief Wrapper class around Boost.Polygon Voronoi generation
 */
class Voronoi_builder
{
public:
  // Attributes

  // Implementation
  /**
   * @brief Construct with given width/height for scaling points
   * @param width Map width
   * @param height Map height
   * @param scale_factor Conversion factor from floating to integer
   */
  Voronoi_builder(double width, double height, double scale_factor);

  /**
   * @brief Build Voronoi cells from given points
   * @param points Input points
   * @return Vector of Voronoi cells
   */
  std::vector<Cell> Build_cells(const std::vector<Point>& points);

  /**
   * @brief Perform Lloyd relaxation on the current Voronoi cells
   * @param iterations Number of iterations to run (1–3 is typical)
   */
  void Relax_cells(int iterations = 1);

  /**
   * @brief Export a simple PPM image of the Voronoi cells
   * @param filename Output filename
   */
  void Export_PPM(const std::string& filename);

private:
  // Attributes
  /**
   * @brief Map width
   */
  double m_width;

  /**
   * @brief Map height
   */
  double m_height;

  /**
   * @brief Scale factor for decimal to int conversions for Boost.Polygon.
   * @details Smaller scale factor means coarser rounding, some points may collapse
   * if decimal precision is removed, and Voronoi cells can become slightly more uniform
   * with less variation in shape and size.
   * A larger scale factor means more precise site positions, cells preserve
   * subtle differences, and slightly more irregular/organic-looking cells.
   */
  double m_scale_factor;

  /**
   * @brief The generated cells
   */
  std::vector<Cell> m_cells;

  // Implementation
  /**
   * @brief Create dummy points for every map point. These dummy points will be
   * used to bridge the boundary on the east/west map edges, allowing those cells
   * to pretend they are adjacent. This allows for:
   * 1. Full voronoi polygon creation of edge cells
   * 2. Smooth transition across the map boundary for adjacent cells
   * @param points The points to create ghosts for
   * @return A vector of all ghost points
   */
  std::vector<Point> world_wrap_ghost_points(const std::vector<Point>& points);

};
}

#endif
