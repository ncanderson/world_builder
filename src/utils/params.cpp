/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <random>

// JSON
#include <deps/json.hpp>

// Application files
#include <utils/params.h>

///////////////////////////////////////////////////////////////////////

using params = world_builder::Params;

///////////////////////////////////////////////////////////////////////

params::Params(std::ifstream params_path)
  :
  m_width(),
  m_height(),
  m_smooth_passes(),
  m_randomness(),
  m_sea_level(),
  m_river_spawn_prob(),
  m_max_river_length(),
  m_seed(std::random_device{}())
{
  nlohmann::json file_data = nlohmann::json::parse(params_path);

  m_width = file_data.at("width");
  m_height = file_data.at("height");
  m_smooth_passes = file_data.at("smooth_passes");
  m_randomness = file_data.at("randomness");
  m_sea_level = file_data.at("sea_level");
  m_river_spawn_prob = file_data.at("river_spawn_prob");
  m_max_river_length = file_data.at("max_river_length");
}

///////////////////////////////////////////////////////////////////////
