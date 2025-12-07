/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef POISSON_DISC_H
#define POISSON_DISC_H

// Standard libs
#include <vector>
#include <string>

// Application files

namespace world_builder
{

/**
 * @brief Simple coord pairs for creating a grid
 */
struct Point
{
  /**
   * @brief x The X coord
   */
  double x;

  /**
   * @brief y The Y coord
   */
  double y;
};

/**
 * @brief Implementation of the Poisson disc sampling algorithm for generating
 * the underlying points to be used in the generated map.
 */
class Poisson_disc
{

public:
  // Attributes

  // Implementation
  /**
   * @brief Constructor
   * @param width
   * @param height
   * @param radius
   * @param attempts
   */
  Poisson_disc(double width, double height, double radius, int attempts = 30);

  /**
   * @brief Generate all points using Poisson disc sampling
   * @return Vector of points generated
   */
  std::vector<Point> Generate();

  /**
   * @brief Save a Poisson_disc points vector as a simple image
   * @param points
   * @param width
   * @param height
   * @param filename
   */
  void Save_points_as_ppm(const std::string& filename);

private:
  // Attributes
  /**
   * @brief Default value for points, to facilitate checks on undefined points
   */
  const int POINT_UNDEFINED = -1;

  /**
   * @brief Map width
   */
  double m_width;

  /**
   * @brief Map height
   */
  double m_height;

  /**
   * @brief Every point placed must be at least `m_radius` units away from all other points
   */
  double m_radius;

  /**
   * @brief Attempts to find a new neighboring point. Higher values will create
   * more points
   */
  int m_k_attempts;

  /**
   * @brief This algorithm creates a grid that overlays the sampling space. Each
   * grid cell is a square with side length of `m_radius / std::sqrt(2.0)` where
   * the radius is the minimum distance. The idea is that if two points are
   * closer than r, they must lie in the same cell or a neighboring cell.
   * Using `m_radius / std::sqrt(2.0)` ensures at most one point per cell, so
   * we never need to scan the entire list of points — just the 3×3 neighborhood
   * of cells.
   */
  double m_cell_size;

  /**
   * @brief Width of the grid. This is computed from m_width and m_radius, to
   * determine how many cells wide the grid is.
   */
  int m_grid_width;

  /**
   * @brief Height of the grid. This is computed from m_height and m_radius, to
   * determine how many cells high the grid is.
   */
  int m_grid_height;

  /**
   * @brief The m_map_grid, a conceptual layer that will help create points
   */
  std::vector<int> m_map_grid;

  /**
   * @brief The m_grid_points produced on this grid
   */
  std::vector<Point> m_grid_points;

  /**
   * @brief The active points that are eligible for neighbors
   */
  std::vector<int> m_active_points;

  // Implementation
  /**
   * @brief Whether a point is within the valid bounds of the map (vertical only)
   * @param p
   * @return
   */
  bool in_bounds(const Point& p);

  /**
   * @brief Wrap x-coordinate around horizontally
   */
  Point wrap_around(const Point& p) const;

  /**
   * @brief Maps a point’s real coordinates to a cell in the overlay grid,
   * wrapping horizontally, and stores its index so neighbor checks are fast.
   * @param index The index of the stored point
   * @param point The point to place in the grid
   */
  void place_in_grid(int index, const Point& point);

  /**
   * @brief Whether this point has neighbors. New points are not valid if they
   * are with m_radius of another point.
   * @param point The point to check
   * @return True if no neighboring points were too close to `point`; candidate is valid.
   * Otherwise false.
   */
  bool no_neighbors(const Point& point);

  /**
   * @brief Picks a random spot somewhere in the ring around `point`, that ring
   * having an inner radius m_radius and outer radius 2*m_radius.
   * @param point
   * @return The new point, wrapped horizontally
   */
  Point random_around(const Point& point);

};
}

#endif
