/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */


// Standard libs
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <boost/program_options.hpp>

// JSON

// Application files
#include <utils/world_builder_utils.h>
#include <utils/params.h>
#include <geo_models/coord.h>
#include <geo_models/continent.h>
#include <geo_models/tile.h>

// Data access layer

///////////////////////////////////////////////////////////////////////
// Global Variables
///////////////////////////////////////////////////////////////////////

// NOOP

///////////////////////////////////////////////////////////////////////
// Function Declarations
///////////////////////////////////////////////////////////////////////

// NOOP

///////////////////////////////////////////////////////////////////////


// Simple RNG
struct RNG
{
  std::mt19937 gen;
  RNG(unsigned seed) : gen(seed) {}

  double uniform(double a=0.0, double b=1.0)
  {
    std::uniform_real_distribution<double> dist(a, b);
    return dist(gen);
  }

  int randint(int a, int b)
  {
    std::uniform_int_distribution<int> dist(a, b);
    return dist(gen);
  }
};

int main(int argc, char *argv[])
{
  //////////////////////////////////////////////////////
  // Config defaults
  std::string app_cfg_path = "/home/nanderson/nate_personal/projects/world_builder/config/gen_params.json";

  //////////////////////////////////////////////////////
  // Set up the program options
  namespace po = boost::program_options;

  // Declare the supported options.
  po::options_description desc("Application options");
  desc.add_options()
      ("help", "Produce help message")
      ("app_cfg", po::value(&app_cfg_path)->default_value(app_cfg_path), "Main application config file");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  //////////////////////////////////////////////////////
  // Parse options

  if(vm.count("help"))
  {
    std::cout << desc << "\n";
    return 1;
  }

  world_builder::Params params;

  if(vm.count("app_cfg"))
  {
    try
    {
      std::ifstream app_cfg_file(app_cfg_path);
      params = world_builder::Params(std::ifstream(app_cfg_path));
    }
    catch (const std::exception& e)
    {
      world_builder::Print_to_cout("Error loading config from JSON");
      world_builder::Print_to_cout(e.what());
      return 1;
    }
  }
  else
  {
    // Error, exit
    world_builder::Print_to_cout("No config found, exiting");
    return 1;
  }

  //////////////////////////////////////////////////////
  // Set up Runtime Objects

  RNG rng(params.Get_seed());

  std::unordered_map<world_builder::Coord, world_builder::Tile, world_builder::Coord_hash> tiles;

  // Using the params, build a grid of Coord objects, which are then used to
  // build a Tile. These tiles represent the individual unit that builds the
  // whole map.
  for (int q = 0; q < params.Get_width(); ++q)
  {
    for (int r = 0; r < params.Get_height(); ++r)
    {
      world_builder::Coord new_coord = world_builder::Coord(q, r);
      tiles.try_emplace(new_coord, world_builder::Tile(new_coord));
    }
  }

  // ----------------------
  // Continent-based seeding
  // ----------------------

  // Define 2–4 continents depending on map size
  int numContinents = std::max(static_cast<uint32_t>(2),
                               params.Get_width() / 40);
  std::vector<world_builder::Continent> continents;
  for (int i = 0; i < numContinents; ++i)
  {
    continents.push_back({
      rng.randint(params.Get_width() / 8, params.Get_width() * 7 / 8),    // centerQ
      rng.randint(params.Get_height() / 8, params.Get_height() * 7 / 8),  // centerR
      rng.uniform(params.Get_width() / 6.0, params.Get_width() / 4.0)     // radius
    });
  }

  // Place elevation seeds around each continent center
  int seedsPerContinent = std::max(static_cast<uint32_t>(3),
                                   (params.Get_width() * params.Get_height()) / (200 * numContinents));
  for (auto& c : continents)
  {
    for (int i = 0; i < seedsPerContinent; ++i)
    {
      double angle = rng.uniform(0, 2 * M_PI);
      double dist = rng.uniform(0, c.Get_radius());
      int q_coord = c.Get_center_q() + static_cast<int>(dist * cos(angle));
      int r_coord = c.Get_center_r() + static_cast<int>(dist * sin(angle));

      if(q_coord >= 0 && q_coord < params.Get_width() && r_coord >= 0 && r_coord < params.Get_height())
      {
        // Skewed toward land
        world_builder::Coord coord = world_builder::Coord(q_coord, r_coord);
        auto it = tiles.find(coord);
        if (it != tiles.end())
        {
          it->second.Set_elevation(rng.uniform(0.4, 1.0));
        }
      }
    }
  }

  // Only add ocean seeds outside continents
  int oceanSeeds = seedsPerContinent; // same count as land seeds
  for (int i = 0; i < oceanSeeds; ++i)
  {
    int q = rng.randint(0, params.Get_width() - 1);
    int r = rng.randint(0, params.Get_height() - 1);

    // skip tiles that are close to a continent center
    bool nearContinent = false;
    for (auto& c : continents)
    {
      double dq = q - c.Get_center_q();
      double dr = r - c.Get_center_r();
      double dist = std::sqrt(dq*dq + dr*dr);
      if (dist < c.Get_radius() * 0.8)
      {
        nearContinent = true;
        break;
      }
    }

    if(!nearContinent)
    {
      // Skewed toward land
      world_builder::Coord coord = world_builder::Coord(q, r);
      auto it = tiles.find(coord);
      if (it != tiles.end())
      {
        it->second.Set_elevation(rng.uniform(-0.5, 0.2));
      }
    }
  }

  // Diffusion / smoothing. For every smoothing pass, this will set the elevation for each
  // to based on the average of all neighbors with some random noise injected.
  for (int pass = 0; pass < params.Get_smooth_passes(); ++pass)
  {
    // Temp map to hold all coordinates, allowing update of the main map after
    // doing the smoothing passes
    std::unordered_map<world_builder::Coord,
                       double,
                       world_builder::Coord_hash> new_elev;

    // ie for every tile...
    for (auto& [coord, tile] : tiles)
    {
      // get all neighbors for this tile
      auto neighbors = tile.Get_neighbor_tiles(coord);

      // Neighboring elevations
      std::vector<double> neighbor_elevations;

      // for all neighbors,
      for (auto& nn : neighbors)
      {
        // Get the pointed to neighbor tile
        auto it2 = tiles.find(nn);
        if(it2 != tiles.end())
        {
          // If found, add the elevation
          neighbor_elevations.push_back(it2->second.Get_elevation());
        }
      }

      // Average all neighboring elevations
      double nbr_mean = 0;

      // If no neighbors, use this tile's own elevation
      if(neighbor_elevations.empty())
      {
        nbr_mean = tile.Get_elevation();
      }
      // Otherwise, add up all neighboring elevations and divide by found neighbors
      else
      {
        nbr_mean = std::accumulate(neighbor_elevations.begin(),
                                   neighbor_elevations.end(), 0.0) / neighbor_elevations.size();
      }

      double blend = 0.6;

      // (rng.uniform() - 0.5): Make the number in the range of -.5 to .5
      // * params.randomness: Augment the random roll with the additional randomness factor
      // (1.0 - (double)pass / params.smooth_passes): Dampens the noise gradually with each smoothing pass.
      double noise = (rng.uniform() - 0.5) * params.Get_randomness() * (1.0 - (double)pass / params.Get_smooth_passes());

      // New elevation is a weighted average between (old elevation) and (neighbor mean), plus some fading noise.
      // If blend = 0.5 → half current height, half neighbors → moderate smoothing.
      // If blend = 1.0 → completely replace with neighbor mean (max smoothing).
      // If blend = 0.0 → do nothing (preserve current map).
      new_elev[coord] = nbr_mean * blend + tile.Get_elevation() * (1 - blend) + noise;
    }

    // Reset the tiles to the new elevation
    for (const auto& [coord, val] : new_elev)
    {
      auto it = tiles.find(coord);
      if (it != tiles.end())
      {
        it->second.Set_elevation(val);
      }
    }
  }

  // Normalize elevation
  double minE = 1e9;
  double maxE = -1e9;

  // For every tile, check for a new max or min elevation
  for(auto& [c, t] : tiles)
  {
    minE = std::min(minE, t.Get_elevation());
    maxE = std::max(maxE, t.Get_elevation());
  }

  // For every tile, re-scale the elevation to fit within a 0 - 1 range
  for(auto& [c, t] : tiles)
  {
    t.Set_elevation((t.Get_elevation() - minE) / (maxE - minE));
  }

  // Terrain classification based on elevation
  for(auto& [c, t] : tiles)
  {
    if(t.Get_elevation() <= params.Get_sea_level())
    {
      t.Set_terrain(world_builder::ETerrain::ETERRAIN_Ocean);
    }
  }

  // Mark coasts
  for(auto& [c, t] : tiles)
  {
    // Ignore oceans
    if(t.Get_terrain() == world_builder::ETerrain::ETERRAIN_Ocean)
    {
      continue;
    }

    // For every tile, if it's not an ocean but a neighbor is an ocean, then
    // this is a coast
    for(auto& n : t.Get_neighbor_tiles(c))
    {
      auto it2 = tiles.find(n);
      if(it2 != tiles.end() && it2->second.Get_terrain() == world_builder::ETerrain::ETERRAIN_Ocean)
      {
        t.Set_is_coast(true);
        break;
      }
    }
  }

  // Rivers
  std::vector<std::vector<world_builder::Coord>> rivers;
  // for every tile,
  for(auto& [c, t] : tiles)
  {
    // Check elevation. greater than sea level (plus a pad), and make a roll against probability
    if(t.Get_elevation() > params.Get_sea_level() + 0.05 && rng.uniform() < params.Get_river_spawn_prob())
    {
      // Trace a river path,
      auto path = t.Trace_river(c, tiles, params);
      // if there are three or more tiles,
      if (path.size() >= 3)
      {
        // for each tile in the river path,
        for(size_t i = 0; i + 1 < path.size(); ++i)
        {
          auto it = tiles.find(path[i]);
          if (it != tiles.end())
          {
            it->second.Set_is_river(true);
            it->second.Set_river_to(path[i + 1]);
          }
        }

        // handle the last tile
        auto it_last = tiles.find(path.back());
        if(it_last != tiles.end())
        {
          it_last->second.Set_is_river(true);
        }

        rivers.push_back(path);
      }
    }
  }

  // Terrain painting
  for (auto& [c, t] : tiles)
  {
    if (t.Get_terrain() == world_builder::ETerrain::ETERRAIN_Ocean)
    {
      continue;
    }

    if(t.Get_is_river())
    {
      t.Set_terrain(world_builder::ETerrain::ETERRAIN_River);
      continue;
    }

    if(t.Get_is_coast() && t.Get_elevation() <= params.Get_sea_level() + 0.03)
    {
      t.Set_terrain(world_builder::ETerrain::ETERRAIN_Beach);
    }
    else if(t.Get_elevation() < params.Get_sea_level() + 0.07)
    {
      t.Set_terrain(world_builder::ETerrain::ETERRAIN_Marsh);
    }
    else if(t.Get_elevation() < params.Get_sea_level() + 0.20)
    {
      t.Set_terrain(world_builder::ETerrain::ETERRAIN_Plains);
    }
    else if(t.Get_elevation() < params.Get_sea_level() + 0.45)
    {
      t.Set_terrain(world_builder::ETerrain::ETERRAIN_Hills);
    }
    else
    {
      if(t.Get_elevation() > 0.8)
      {
        t.Set_terrain(world_builder::ETerrain::ETERRAIN_Mountains);
      }
      else
      {
        t.Set_terrain(world_builder::ETerrain::ETERRAIN_Hills);
      }
    }
  }

  // Print summary
  std::cout << "World generated (" << params.Get_width() << "x" << params.Get_height()
            << "), rivers: " << rivers.size() << "\n";
  std::unordered_map<std::string_view, int> counts;

  std::string_view terrain_string;

  for(auto& [c, t] : tiles)
  {
    terrain_string = world_builder::Enum_to_string<world_builder::ETerrain>(t.Get_terrain(),
                                                                            world_builder::TERRAIN_LOOKUP);
    counts[terrain_string]++;
  }

  for(auto& [name, n] : counts)
  {
    std::cout << "  " << name << ": " << n << "\n";
  }

  // -------------------------------------------------------------
  // HTML VISUALIZATION OUTPUT
  // -------------------------------------------------------------
  {
    // Create ./output directory if missing
    std::filesystem::path outputDir = std::filesystem::path(PROJECT_ROOT_DIR) / "output";
    std::filesystem::create_directories(outputDir);
    std::filesystem::path outputFile = outputDir / "index.html";
    std::ofstream html(outputFile);

    if(!html)
    {
      std::cerr << "Failed to open index.html for writing.\n";
    }
    else
    {
      int scale = 10; // change this to make each tile bigger/smaller
      html << R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>World Preview</title>
<style>
  html, body {
    margin: 0;
    padding: 0;
    background: #111;
    height: 100%;
    overflow: hidden;
    display: flex;
    justify-content: center;
    align-items: center;
  }
  #container {
    position: relative;
    width: 100%;
    height: 100%;
  }
  canvas {
    image-rendering: pixelated;
    width: 100%;
    height: 100%;
    object-fit: contain;
    border: 1px solid #333;
  }
  #legend {
    position: absolute;
    top: 10px;
    left: 10px;
    font-family: monospace;
    color: #eee;
    background: rgba(0,0,0,0.5);
    padding: 6px 10px;
    border-radius: 6px;
  }
  .swatch {
    display: inline-block;
    width: 12px;
    height: 12px;
    margin-right: 6px;
    vertical-align: middle;
  }
</style>
</head>
<body>
<div id="container">
  <div id="legend">
    <div><span class="swatch" style="background:#004;"></span>Ocean</div>
    <div><span class="swatch" style="background:#66f;"></span>River</div>
    <div><span class="swatch" style="background:#eeddaa;"></span>Beach</div>
    <div><span class="swatch" style="background:#88aa55;"></span>Plains</div>
    <div><span class="swatch" style="background:#557744;"></span>Hills</div>
    <div><span class="swatch" style="background:#999999;"></span>Mountains</div>
  </div>
  <canvas id="map" width=)" << params.Get_width() << " height=" << params.Get_height() << R"(></canvas>
</div>
<script>
const canvas = document.getElementById('map');
const ctx = canvas.getContext('2d');
const img = ctx.createImageData(canvas.width, canvas.height);
const data = img.data;
)";

      // Write out tile array
      html << "const tiles = [\n";
      for (int r = 0; r < params.Get_height(); ++r)
      {
        for (int q = 0; q < params.Get_width(); ++q)
        {
          const world_builder::Tile& t = tiles.at({q, r});
          std::string_view terrain_string = world_builder::Enum_to_string<world_builder::ETerrain>(t.Get_terrain(),
                                                                                                   world_builder::TERRAIN_LOOKUP);
          html << "{e:" << t.Get_elevation() << ",t:'" << terrain_string << "'},";
        }
        html << "\n";
      }
      html << "];\n";

      html << R"(
// Render map pixels
for (let y = 0; y < canvas.height; ++y) {
  for (let x = 0; x < canvas.width; ++x) {
    const i = y * canvas.width + x;
    const tile = tiles[i];
    let r,g,b;
    switch(tile.t) {
      case 'ocean':     r=0; g=0; b=100 + tile.e*100; break;
      case 'river':     r=80; g=120; b=255; break;
      case 'beach':     r=238; g=214; b=175; break;
      case 'marsh':     r=110; g=150; b=90; break;
      case 'plains':    r=136; g=170; b=85; break;
      case 'hills':     r=85; g=119; b=68; break;
      case 'mountain':  r=150 + tile.e*100; g=150 + tile.e*100; b=150 + tile.e*100; break;
      default:          r=g=b=50;
    }
    const j = ((canvas.height - 1 - y) * canvas.width + x) * 4; // flip vertically
    data[j] = r;
    data[j+1] = g;
    data[j+2] = b;
    data[j+3] = 255;
  }
}
ctx.putImageData(img, 0, 0);
</script>
</body>
</html>
)";

      html.close();
      std::cout << "HTML world map written to index.html (open in browser)\n";
    }
  }


  return 0;
}
