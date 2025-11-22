/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <fstream>

// JSON

// Application files
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
  // Clear previous cells
  m_cells.clear();

  // Convert floating point sites to boost::polygon::point_data<double>
  std::vector<point_data<double>> boost_points;
  boost_points.reserve(points.size());
  for (const auto& p : points)
  {
    // Scale using the configured scale factor
    boost_points.emplace_back(p.x * m_scale_factor, p.y * m_scale_factor);
  }

  // Use boost API
  voronoi_diagram<double> vd;
  construct_voronoi(boost_points.begin(), boost_points.end(), &vd);

  // Convert diagram edges to polygon cells
  for (auto it = vd.cells().begin(); it != vd.cells().end(); ++it)
  {
    Cell cell;
    int idx = it->source_index();
    cell.site = points[idx];

    const auto* edge = it->incident_edge();
    if (!edge)
    {
      continue;
    }

    const auto* start = edge;
    for (const auto* e = start; e != nullptr; e = e->next())
    {
      if (e->is_primary())
      {
        Point v0{
            static_cast<double>(e->vertex0()->x()) / m_scale_factor,
            static_cast<double>(e->vertex0()->y()) / m_scale_factor
        };
        Point v1{
            static_cast<double>(e->vertex1()->x()) / m_scale_factor,
            static_cast<double>(e->vertex1()->y()) / m_scale_factor
        };
        cell.vertices.push_back(v0);
        cell.vertices.push_back(v1);
      }

      if (e->next() == start)
        break;
    }

    m_cells.push_back(cell);
  }

  return m_cells;
}

///////////////////////////////////////////////////////////////////////

void vb::Export_PPM(const std::string& filename)
{
  // Create an image buffer filled with black
  std::vector<std::vector<std::array<unsigned char, 3>>> image(m_height,
                                                               std::vector<std::array<unsigned char,3>>(m_width, {0,0,0}));

  // Create random colors for filling out Voronoi cells
  std::vector<std::array<unsigned char, 3>> colors;
  for (size_t i = 0; i < m_cells.size(); ++i)
  {
    colors.push_back(dice::Create_random_color());
  }

  // Fill pixels with nearest site color
  for (int y = 0; y < m_height; ++y)
  {
    for (int x = 0; x < m_width; ++x)
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

      image[y][x] = colors[nearest];
    }
  }

  // Write PPM file
  std::ofstream ofs(filename, std::ios::out);
  ofs << "P3\n" << m_width << " " << m_height << "\n255\n";
  for (int y = 0; y < m_height; ++y)
  {
    for (int x = 0; x < m_width; ++x)
    {
      auto& c = image[y][x];
      ofs << (int)c[0] << " " << (int)c[1] << " " << (int)c[2] << " ";
    }
    ofs << "\n";
  }
  ofs.close();
}

///////////////////////////////////////////////////////////////////////
