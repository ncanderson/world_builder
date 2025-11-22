/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <cmath>
#include <fstream>

// JSON

// Application files
#include <geo_models/voronoi/poisson_disc.h>
#include <defs/dice_rolls.h>

///////////////////////////////////////////////////////////////////////

using pd = world_builder::Poisson_disc;

///////////////////////////////////////////////////////////////////////

pd::Poisson_disc(double width, double height, double radius, int attempts)
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

///////////////////////////////////////////////////////////////////////

std::vector<world_builder::Point> pd::Generate()
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

///////////////////////////////////////////////////////////////////////

void pd::Save_points_as_ppm(const std::string& filename)
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

///////////////////////////////////////////////////////////////////////

bool pd::in_bounds(const world_builder::Point& p)
{
  return (p.y >= 0 && p.y < m_height);
}

///////////////////////////////////////////////////////////////////////

world_builder::Point pd::wrap_around(const Point& p) const
{
  double x = std::fmod(p.x, m_width);
  if (x < 0) x += m_width;
  return Point{x, p.y};
}

///////////////////////////////////////////////////////////////////////

void pd::place_in_grid(int index, const Point& point)
{
  // Wrap the point
  Point wrapped_point = wrap_around(point);

  // Convert the continuous x, y position of the point into grid cell coordinates.
  int grid_x = int(wrapped_point.x / m_cell_size);
  int grid_y = int(wrapped_point.y / m_cell_size);

  // Convert the 2D coordinates into a 1D index for the flat vector m_map_grid
  m_map_grid[grid_y * m_grid_width + grid_x] = index;
}

///////////////////////////////////////////////////////////////////////

bool pd::no_neighbors(const Point& point)
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

///////////////////////////////////////////////////////////////////////

world_builder::Point pd::random_around(const Point& point)
{
  double a = dice::Make_a_roll<double>(0, 2*M_PI);
  double r = dice::Make_a_roll<double>(m_radius, 2*m_radius);
  Point new_point{point.x + r * std::cos(a), point.y + r * std::sin(a)};
  return wrap_around(new_point);
}

///////////////////////////////////////////////////////////////////////
