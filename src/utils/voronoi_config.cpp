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
  m_width(),
  m_height(),
  m_min_distance(),
  m_k_attempts(),
  m_voronoi_scale_factor(),
  m_relax_iterations()
{
  nlohmann::json file_data = nlohmann::json::parse(params_path);
  m_width = file_data.value("map_width", m_width);
  m_height = file_data.value("map_height", m_height);
  m_min_distance = file_data.value("point_min_distance", m_min_distance);
  m_k_attempts = file_data.value("point_attempts", m_k_attempts);
  m_voronoi_scale_factor = file_data.value("voronoi_scale_factor",
                                           m_voronoi_scale_factor);
  m_relax_iterations = file_data.value("cell_relaxations", m_relax_iterations);
}

///////////////////////////////////////////////////////////////////////

voronoi::Voronoi_config()
{ }

///////////////////////////////////////////////////////////////////////
