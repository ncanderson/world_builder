/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <fstream>

// Application files
#include <utils/world_builder_utils.h>
#include <geo_models/voronoi/voronoi_builder.h>

///////////////////////////////////////////////////////////////////////

using vb = world_builder::Voronoi_builder;

///////////////////////////////////////////////////////////////////////

vb::Voronoi_builder(double width, double height, double scale_factor)
    : m_width(width),
    m_height(height),
    m_scale_factor(scale_factor),
    m_cells()
{ }

///////////////////////////////////////////////////////////////////////

std::vector<world_builder::Cell> vb::Build_cells(const std::vector<Point>& incoming)
{
  m_cells.clear();

  //------------------------------------------------------------------
  // 1. Detect if input is original-only or already ghost-expanded
  //------------------------------------------------------------------
  bool incoming_extended = false;
  for (const auto& p : incoming)
  {
    if (p.x < 0.0 || p.x >= m_width)
    {
      incoming_extended = true;
      break;
    }
  }

  //------------------------------------------------------------------
  // 2. Rebuild ghosted point list
  //------------------------------------------------------------------
  std::vector<Point> pts;
  std::vector<bool> is_real;

  if (!incoming_extended)
  {
    // Save originals in canonical order
    m_original_points = incoming;

    // WRAP FIX:
    // Use full left+right tiling. No bounding radius heuristics.
    pts.reserve(incoming.size() * 3);
    is_real.reserve(incoming.size() * 3);

    for (const auto& p : incoming)
    {
      // 1. Original
      pts.push_back(p);
      is_real.push_back(true);

      // 2. Ghost: left tile (p.x - width)
      pts.push_back(Point{p.x - m_width, p.y});
      is_real.push_back(false);

      // 3. Ghost: right tile (p.x + m_width)
      pts.push_back(Point{p.x + m_width, p.y});
      is_real.push_back(false);
    }
  }
  else
  {
    // Already extended: trust ordering (originals first)
    pts = incoming;
    is_real.reserve(pts.size());

    for (const auto& p : pts)
      is_real.push_back(p.x >= 0.0 && p.x < m_width);

    // recover originals
    m_original_points.clear();
    for (size_t i = 0; i < pts.size(); i++)
      if (is_real[i])
        m_original_points.push_back(pts[i]);
  }

  const size_t N = m_original_points.size();

  //------------------------------------------------------------------
  // 3. Convert to scaled Boost points
  //------------------------------------------------------------------
  std::vector<point_data<double>> boost_pts;
  boost_pts.reserve(pts.size());

  for (const auto& p : pts)
    boost_pts.emplace_back(p.x * m_scale_factor, p.y * m_scale_factor);

  //------------------------------------------------------------------
  // 4. Build diagram
  //------------------------------------------------------------------
  voronoi_diagram<double> vd;
  construct_voronoi(boost_pts.begin(), boost_pts.end(), &vd);

  //------------------------------------------------------------------
  // 5. Prepare output slots
  //------------------------------------------------------------------
  std::vector<Cell> result(N);
  std::vector<bool> filled(N, false);

  //------------------------------------------------------------------
  // 6. Convert Voronoi cells into polygons
  //------------------------------------------------------------------
  for (const auto& c : vd.cells())
  {
    int idx = c.source_index();
    if (idx < 0 || idx >= (int)pts.size())
      continue;

    if (!is_real[idx])
      continue;

    // Determine original index
    int real_index = 0;
    for (int i = 0; i < idx; i++)
      if (is_real[i])
        real_index++;

    int orig = real_index;
    if (orig < 0 || orig >= (int)N)
      continue;

    Cell out;
    out.site = pts[idx];
    out.id   = orig;
    out.color = dice::Create_random_color();

    const auto* e = c.incident_edge();
    if (!e)
    {
      result[orig] = std::move(out);
      filled[orig] = true;
      continue;
    }

    const auto* start = e;
    std::vector<Point> poly;
    poly.reserve(16);

    do
    {
      if (e->is_primary() && e->vertex0())
      {
        double vx = e->vertex0()->x() / m_scale_factor;
        double vy = e->vertex0()->y() / m_scale_factor;

        // VERTEX FIX: wrap ONLY horizontally into the base domain
        if (vx < 0)      vx += m_width;
        if (vx >= m_width) vx -= m_width;

        poly.push_back(Point{vx, vy});
      }

      e = e->next();
    }
    while (e != start);

    out.vertices = std::move(poly);
    result[orig] = std::move(out);
    filled[orig] = true;
  }

  //------------------------------------------------------------------
  // 7. Ensure stable ordering
  //------------------------------------------------------------------
  m_cells.clear();
  m_cells.reserve(N);

  for (size_t i = 0; i < N; i++)
  {
    if (filled[i])
    {
      m_cells.push_back(std::move(result[i]));
    }
    else
    {
      // placeholder
      Cell c;
      c.site = m_original_points[i];
      c.id = i;
      c.color = dice::Create_random_color();
      m_cells.push_back(std::move(c));
    }
  }

  return m_cells;
}

///////////////////////////////////////////////////////////////////////

void vb::Relax_cells(int iterations)
{
  for (int step = 0; step < iterations; step++)
  {
    std::vector<Point> wrapped = world_wrap_points(m_original_points);
    Build_cells(wrapped);

    std::vector<Point> new_orig;
    new_orig.reserve(m_original_points.size());

    for (size_t i = 0; i < m_original_points.size(); i++)
    {
      const Cell& c = m_cells[i];
      if (c.vertices.empty())
      {
        new_orig.push_back(c.site);
        continue;
      }

      double sx = 0, sy = 0;
      for (const auto& v : c.vertices)
      {
        double vx = v.x;

        // centroid wrap relative to site
        double dx = vx - c.site.x;
        if (dx >  m_width * 0.5) vx -= m_width;
        if (dx < -m_width * 0.5) vx += m_width;

        sx += vx;
        sy += v.y;
      }

      Point cen{ sx / c.vertices.size(), sy / c.vertices.size() };

      // wrap horizontally only
      if (cen.x < 0)      cen.x += m_width;
      if (cen.x >= m_width) cen.x -= m_width;

      new_orig.push_back(cen);
    }

    m_original_points = std::move(new_orig);
  }

  Build_cells(world_wrap_points(m_original_points));
}

///////////////////////////////////////////////////////////////////////

void vb::Export_PPM(const std::string& filename)
{
  int img_width  = static_cast<int>(m_width);
  int img_height = static_cast<int>(m_height);

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

  // Image buffer (black)
  std::vector<std::vector<std::array<unsigned char, 3>>> image(
      img_height,
      std::vector<std::array<unsigned char, 3>>(img_width, {0,0,0})
      );

  // --- nearest-site fill ----------------------------------------------------
  for (int y = 0; y < img_height; ++y)
  {
    for (int x = 0; x < img_width; ++x)
    {
      double min_dist = std::numeric_limits<double>::max();
      size_t nearest = 0;

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

  // --- draw Poisson sites on top --------------------------------------------
  auto draw_point = [&](int cx,
                        int cy,
                        int radius,
                        unsigned char r,
                        unsigned char g,
                        unsigned char b)
  {
    int r2 = radius * radius;
    for (int dy = -radius; dy <= radius; ++dy)
    {
      for (int dx = -radius; dx <= radius; ++dx)
      {
        if (dx*dx + dy*dy > r2) continue;

        int px = cx + dx;
        int py = cy + dy;

        if (px >= 0 && px < img_width &&
            py >= 0 && py < img_height)
        {
          image[py][px] = {r, g, b};
        }
      }
    }
  };

  // Bright white point marker
  const unsigned char PR = 255, PG = 255, PB = 255;
  const int point_radius = 2;

  for (const auto& cell : m_cells)
  {
    draw_point(
        static_cast<int>(cell.site.x),
        static_cast<int>(cell.site.y),
        point_radius,
        PR, PG, PB
        );
  }

  // --- write PPM -------------------------------------------------------------
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
      ofs << int(c[0]) << " " << int(c[1]) << " " << int(c[2]) << " ";
    }
    ofs << "\n";
  }
  ofs.close();
}

///////////////////////////////////////////////////////////////////////

std::vector<world_builder::Point> vb::world_wrap_points(const std::vector<Point>& pts)
{
  // GHOST FIX:
  // Always generate full L + C + R horizontal tiling.
  std::vector<Point> out;
  out.reserve(pts.size() * 3);

  for (const auto& p : pts)
  {
    // original
    out.push_back(p);

    // left tile
    out.push_back(Point{p.x - m_width, p.y});

    // right tile
    out.push_back(Point{p.x + m_width, p.y});
  }

  return out;
}

///////////////////////////////////////////////////////////////////////
