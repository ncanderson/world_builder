/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef POISSON_DISC_H
#define POISSON_DISC_H

// Standard libs
#include <cmath>
#include <fstream>
#include <vector>

// Application files
#include <defs/dice_rolls.h>

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
  Poisson_disc(double width, double height, double radius, int attempts = 30)
    :
    m_width(width),
    m_height(height),
    m_radius(radius),
    m_k_attempts(attempts),
    m_cell_size(),
    m_grid_width(),
    m_grid_height(),
    m_map_grid(),
    m_grid_points(),
    m_active_points()
  {
    // Individual cell size
    m_cell_size = m_radius / std::sqrt(2.0);

    // Number of cells in the grid, as a function of height/width and cell size
    m_grid_width = static_cast<int>(std::ceil(m_width / m_cell_size));
    m_grid_height = static_cast<int>(std::ceil(m_height / m_cell_size));

    // Pre-size the vector with default, -1 values.
    m_map_grid.assign(m_grid_width * m_grid_height, POINT_UNDEFINED);
  }

  /**
   * @brief Generate all points using Poisson disc sampling
   * @return Vector of points generated
   */
  std::vector<Point> Generate()
  {
    // Pick first random point using dice
    Point first{dice::Make_a_roll<double>(0, m_width),
                dice::Make_a_roll<double>(0, m_height)};

    // Add to the grid
    m_grid_points.push_back(first);

    // Add to 'active', meaning this point can have potential neighboring points
    m_active_points.push_back(0);

    // Place this point in the grid
    place_in_grid(0, first);

    // Process active list. New points will be added to this vector, and the
    // loop will continue until no new points can be added
    while (!m_active_points.empty())
    {
      // Random index from active points
      int index = dice::Make_a_roll<int>(0, static_cast<int>(m_active_points.size()) - 1);
      // The specific index of a point (from m_grid_points) that was cached in m_active_points
      int point_index = m_active_points[index];
      // The actual point
      Point p = m_grid_points[point_index];

      // A new point was found
      bool found = false;

      // Try k random samples
      for (int i = 0; i < m_k_attempts; i++)
      {
        // Create a new point
        Point new_point = random_around(p);

        // In bounds and not too far
        if (in_bounds(new_point) && no_neighbors(new_point))
        {
          // Add the point
          m_grid_points.push_back(new_point);
          // Cache the new point's index
          m_active_points.push_back(static_cast<int>(m_grid_points.size()) - 1);
          // Put it in the grid
          place_in_grid(static_cast<int>(m_grid_points.size()) - 1, new_point);
          // Flag
          found = true;
        }
      }

      if (!found)
      {
        // Point not valid, remove the source point from the active points
        // Copy the last element over the element we want to remove
        m_active_points[index] = m_active_points.back();
        // The last element is now a duplicate, so just pop
        m_active_points.pop_back();
      }
    }

    return m_grid_points;
  }

  /**
   * @brief Save a Poisson_disc points vector as a simple image
   * @param points
   * @param width
   * @param height
   * @param filename
   */
  void Save_points_as_ppm(const std::string& filename)
  {
    std::ofstream ofs(filename);
    ofs << "P3\n" << m_width << " " << m_height << "\n255\n";

    // Create a blank white canvas
    std::vector<std::vector<int>> canvas(m_height,
                                         std::vector<int>(m_width, 255));

    // Draw black dots for each point
    for (const auto& p : m_grid_points)
    {
      int ix = static_cast<int>(p.x);
      int iy = static_cast<int>(p.y);
      if (ix >= 0 && ix < m_width && iy >= 0 && iy < m_height)
      {
        // black dot
        canvas[iy][ix] = 0;
      }
    }

    // Write out RGB values
    for (int y = 0; y < m_height; ++y)
    {
      for (int x = 0; x < m_width; ++x)
      {
        ofs << canvas[y][x] << " " << canvas[y][x] << " " << canvas[y][x] << "\n";
      }
    }
  }

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
  bool in_bounds(const Point& p)
  {
    return (p.y >= 0 && p.y < m_height);
  }

  /**
   * @brief Wrap x-coordinate around horizontally
   */
  Point wrap_around(const Point& p) const
  {
    double x = std::fmod(p.x, m_width);
    if (x < 0) x += m_width;
    return Point{x, p.y};
  }

  /**
   * @brief Maps a point’s real coordinates to a cell in the overlay grid,
   * wrapping horizontally, and stores its index so neighbor checks are fast.
   * @param index The index of the stored point
   * @param point The point to place in the grid
   */
  void place_in_grid(int index, const Point& point)
  {
    // Wrap the point
    Point wrapped_point = wrap_around(point);

    // Convert the continuous x, y position of the point into grid cell coordinates.
    int grid_x = int(wrapped_point.x / m_cell_size);
    int grid_y = int(wrapped_point.y / m_cell_size);

    // Convert the 2D coordinates into a 1D index for the flat vector m_map_grid
    m_map_grid[grid_y * m_grid_width + grid_x] = index;
  }

  /**
   * @brief Whether this point has neighbors
   * @param point
   * @return True if no neighboring points were too close to `point`; candidate is valid.
   * Otherwise false.
   */
  bool no_neighbors(const Point& point)
  {
    // Wrap the point horizontally to handle cylindrical map behavior
    // Ensures that points near the left edge correctly consider neighbors near the right edge
    Point wrapped_point = wrap_around(point);

    // Convert the continuous (x, y) coordinates of the point into discrete grid cell coordinates
    // Each grid cell has side length m_cell_size, computed as radius / sqrt(2)
    int grid_x = int(wrapped_point.x / m_cell_size);
    int grid_y = int(wrapped_point.y / m_cell_size);

    // Loop over the 5x5 neighborhood of cells centered on the candidate cell
    // -2..2 is used because points can affect neighbors up to 2 cells away due to the cell size
    // This ensures we check all possible points that could violate the minimum distance
    for (int y = -2; y <= 2; y++)
    {
      for (int x = -2; x <= 2; x++)
      {
        // Wrap x horizontally so that left/right neighbors wrap around the map
        int nx = (grid_x + x + m_grid_width) % m_grid_width;

        // Vertical wrapping is not applied, so check bounds normally
        int ny = grid_y + y;

        // Skip cells that are outside the vertical bounds
        if (ny < 0 || ny >= m_grid_height)
        {
          continue;
        }

        // Get the index of any existing point in this grid cell
        int idx = m_map_grid[ny * m_grid_width + nx];

        // If the cell contains a point
        if (idx != POINT_UNDEFINED)
        {
          // Compute horizontal distance to candidate
          double dx = m_grid_points[idx].x - wrapped_point.x;

          // Wrap horizontal distance across map boundaries
          // e.g., a point near the left edge may be very close to one on the right edge
          if (dx > m_width / 2)
          {
            dx -= m_width;
          }
          if (dx < -m_width / 2)
          {
            dx += m_width;
          }

          // Vertical distance (no wrapping)
          double dy = m_grid_points[idx].y - wrapped_point.y;

          // Check if distance is less than minimum allowed (radius)
          // Using squared distance avoids a square root for efficiency
          if (dx*dx + dy*dy < m_radius * m_radius)
          {
            // Candidate point is too close to an existing point
            return false;
          }
        }
      }
    }
    // No neighboring points were too close; candidate is valid
    return true;
  }

  /**
   * @brief Picks a random spot somewhere in the ring around `point`, that ring
   * having an inner radius m_radius and outer radius 2*m_radius.
   * @param point
   * @return The new point, wrapped horizontally
   */
  Point random_around(const Point& point)
  {
    double a = dice::Make_a_roll<double>(0, 2*M_PI);
    double r = dice::Make_a_roll<double>(m_radius, 2*m_radius);
    Point new_point{point.x + r * std::cos(a), point.y + r * std::sin(a)};
    return wrap_around(new_point);
  }

};
}

#endif
