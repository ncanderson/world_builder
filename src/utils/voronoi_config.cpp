 /**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs

 // JSON
#include <deps/json.hpp>

 // Application files
#include <utils/voronoi_config.h>

///////////////////////////////////////////////////////////////////////

using voronoi = world_builder::Voronoi_config;

///////////////////////////////////////////////////////////////////////

voronoi::Voronoi_config(std::ifstream params_path)
    :
    m_seed(0),
    m_resolution(0.08),
    m_outfile_ext(".png"),
    m_outfile("output" + m_outfile_ext),
    m_erosion_amount(-1.0),
    m_erosion_iterations(3),
    m_num_cities(-1),
    m_num_towns(-1),
    m_image_width(1920),
    m_image_height(1080),
    m_default_extents_height(20.0),
    m_draw_scale(1.0),
    m_enable_slopes(true),
    m_enable_rivers(true),
    m_enable_contour(true),
    m_enable_borders(true),
    m_enable_cities(true),
    m_enable_towns(true),
    m_enable_labels(true),
    m_enable_area_labels(true),
    m_verbose(false)
{
  nlohmann::json file_data = nlohmann::json::parse(params_path);

  m_seed = file_data.value("seed", m_seed);
  m_resolution = file_data.value("resolution", m_resolution);
  m_outfile_ext = file_data.value("outfile_ext", m_outfile_ext);
  m_outfile = file_data.value("outfile", m_outfile);
  m_erosion_amount = file_data.value("erosion_amount", m_erosion_amount);
  m_erosion_iterations = file_data.value("erosion_iterations", m_erosion_iterations);
  m_num_cities = file_data.value("num_cities", m_num_cities);
  m_num_towns = file_data.value("num_towns", m_num_towns);
  m_image_width = file_data.value("image_width", m_image_width);
  m_image_height = file_data.value("image_height", m_image_height);
  m_default_extents_height = file_data.value("default_extents_height", m_default_extents_height);
  m_draw_scale = file_data.value("draw_scale", m_draw_scale);

  m_enable_slopes = file_data.value("enable_slopes", m_enable_slopes);
  m_enable_rivers = file_data.value("enable_rivers", m_enable_rivers);
  m_enable_contour = file_data.value("enable_contour", m_enable_contour);
  m_enable_borders = file_data.value("enable_borders", m_enable_borders);
  m_enable_cities = file_data.value("enable_cities", m_enable_cities);
  m_enable_towns = file_data.value("enable_towns", m_enable_towns);
  m_enable_labels = file_data.value("enable_labels", m_enable_labels);
  m_enable_area_labels = file_data.value("enable_area_labels", m_enable_area_labels);

  m_verbose = file_data.value("verbose", m_verbose);
}

///////////////////////////////////////////////////////////////////////

voronoi::Voronoi_config()
{ }

///////////////////////////////////////////////////////////////////////
