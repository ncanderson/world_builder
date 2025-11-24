/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <fstream>

// JSON

// Application files
#include <utils/world_builder_utils.h>
#include <geo_models/voronoi/voronoi_builder.h>

///////////////////////////////////////////////////////////////////////

using vb = world_builder::Voronoi_builder;

///////////////////////////////////////////////////////////////////////

vb::Voronoi_builder(double width, double height, double scale_factor)
    :
    m_width(width),
    m_height(height),
    m_scale_factor(scale_factor),
    m_cells()
{ }

///////////////////////////////////////////////////////////////////////

std::vector<world_builder::Cell> vb::Build_cells(const std::vector<Point>& points)
{
  // Preserve IDs/colors if we are rebuilding after a relaxation pass and
  // the incoming points vector has the same size/order as the current cells.
  std::vector<int> preserved_ids;
  std::vector<std::array<unsigned char, 3>> preserved_colors;
  const bool preserve = (m_cells.size() == points.size() && !m_cells.empty());
  if (preserve)
  {
    preserved_ids.reserve(m_cells.size());
    preserved_colors.reserve(m_cells.size());
    for (const auto& c : m_cells)
    {
      preserved_ids.push_back(c.id);
      preserved_colors.push_back(c.color);
    }
  }

  m_cells.clear();

  // Use points directly – no wrapping or duplication
  std::vector<point_data<double>> boost_points;
  boost_points.reserve(points.size());
  for (const auto& p : points)
  {
    boost_points.emplace_back(p.x * m_scale_factor, p.y * m_scale_factor);
  }

  voronoi_diagram<double> vd;
  construct_voronoi(boost_points.begin(), boost_points.end(), &vd);

  // Convert edges to cells
  int next_id = 0;
  for (auto it = vd.cells().begin(); it != vd.cells().end(); ++it)
  {
    int idx = it->source_index();
    if (idx < 0 || idx >= points.size())
    {
      continue;
    }

    Cell cell;
    cell.site = points[idx];

    // Restore preserved id/color if available, otherwise assign new ones
    if (preserve)
    {
      cell.id = preserved_ids[idx];
      cell.color = preserved_colors[idx];
    }
    else
    {
      cell.id = next_id++;
      cell.color = dice::Create_random_color();
    }

    const auto* edge = it->incident_edge();
    if (!edge)
    {
      continue;
    }

    const auto* start = edge;
    const auto* e = start;
    do
    {
      if (e->is_primary() && e->vertex0() && e->vertex1())
      {
        Point v0{ e->vertex0()->x() / m_scale_factor,
                 e->vertex0()->y() / m_scale_factor };

        Point v1{ e->vertex1()->x() / m_scale_factor,
                 e->vertex1()->y() / m_scale_factor };

        cell.vertices.push_back(v0);
        cell.vertices.push_back(v1);
      }

      e = e->next();
    } while (e != start);

    m_cells.push_back(cell);
  }

  return m_cells;
}

///////////////////////////////////////////////////////////////////////

void vb::Relax_cells(int iterations)
{
  for (int iter = 0; iter < iterations; ++iter)
  {
    std::vector<Point> new_sites;
    new_sites.reserve(m_cells.size());

    for (const auto& cell : m_cells)
    {
      if (cell.vertices.empty())
      {
        // Keep original site if no vertices
        new_sites.push_back(cell.site);
        continue;
      }

      // Simple Euclidean centroid (no wrapping)
      double sum_x = 0.0;
      double sum_y = 0.0;
      int count = 0;

      for (const auto& v : cell.vertices)
      {
        sum_x += v.x;
        sum_y += v.y;
        ++count;
      }

      Point centroid{ sum_x / count, sum_y / count };

      new_sites.push_back(centroid);
    }

    // Rebuild Voronoi with new centroid positions (Build_cells will preserve IDs/colors)
    Build_cells(new_sites);
  }
}

///////////////////////////////////////////////////////////////////////

void vb::Export_PPM(const std::string& filename)
{
  // Convert member attributes to integers for image dimensions
  int img_width = static_cast<int>(m_width);
  int img_height = static_cast<int>(m_height);

  // Sanity check
  if (img_width <= 0 || img_height <= 0)
  {
    std::cerr << "PPM export: invalid image dimensions\n";
    return;
  }

  if (m_cells.empty())
  {
    std::cerr << "PPM export: no Voronoi cells to draw\n";
    return;
  }

  // Create image buffer filled with black
  std::vector<std::vector<std::array<unsigned char, 3>>> image(img_height,
                                                               std::vector<std::array<unsigned char, 3>>(img_width,
                                                                                                         {0, 0, 0}));

  // Fill pixels with nearest site color
  for (int y = 0; y < img_height; ++y)
  {
    for (int x = 0; x < img_width; ++x)
    {
      double min_dist = std::numeric_limits<double>::max();
      size_t nearest = 0;

      // Find nearest site
      for (size_t i = 0; i < m_cells.size(); ++i)
      {
        double dx = m_cells[i].site.x - x;
        double dy = m_cells[i].site.y - y;
        double d2 = dx*dx + dy*dy;
        if (d2 < min_dist)
        {
          min_dist = d2;
          nearest = i;
        }
      }

      image[y][x] = m_cells[nearest].color;
    }
  }

  // Write PPM file
  std::ofstream ofs(filename, std::ios::out);
  if (!ofs)
  {
    world_builder::Print_to_cout("PPM export: failed to open file " + filename);
    return;
  }

  ofs << "P3\n" << img_width << " " << img_height << "\n255\n";
  for (int y = 0; y < img_height; ++y)
  {
    for (int x = 0; x < img_width; ++x)
    {
      auto& c = image[y][x];
      ofs << static_cast<int>(c[0]) << " "
          << static_cast<int>(c[1]) << " "
          << static_cast<int>(c[2]) << " ";
    }
    ofs << "\n";
  }
  ofs.close();
}

///////////////////////////////////////////////////////////////////////
