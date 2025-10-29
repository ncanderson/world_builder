/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */


// Standard libs
#include <boost/program_options.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

// JSON

// Application files
#include <defs/dice_rolls.h>
#include <geo_models/continent.h>
#include <geo_models/coord.h>
#include <geo_models/tile.h>
#include <geo_models/world.h>
#include <utils/html_writer.h>
#include <utils/params.h>
#include <utils/world_builder_utils.h>

///////////////////////////////////////////////////////////////////////

void Print_output_summary(const world_builder::World& world,
                          const world_builder::Params& params)
{
  std::cout << "World generated (" << params.Get_width() << "x" << params.Get_height()
            << "), rivers: " << world.Get_rivers().size() << "\n";

  // Counts of terrain types
  std::unordered_map<std::string_view, int> counts;

  std::string_view terrain_string;

  for(auto& [c, t] : world.Get_world_tiles())
  {
    terrain_string = world_builder::Enum_to_string<world_builder::ETerrain>(t.Get_terrain(),
                                                                            world_builder::TERRAIN_LOOKUP);
    counts[terrain_string]++;
  }

  for(auto& [name, n] : counts)
  {
    std::cout << "  " << name << ": " << n << "\n";
  }
}

///////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  //////////////////////////////////////////////////////
  // Config defaults
  std::string app_cfg_path = "/home/nanderson/nate_personal/projects/world_builder/config/gen_params.json";

  //////////////////////////////////////////////////////
  // Set up Runtime Objects

  world_builder::Params params;
  world_builder::HTML_writer writer(std::filesystem::path(PROJECT_ROOT_DIR) / "output");

  //////////////////////////////////////////////////////
  // Set up the program options
  namespace po = boost::program_options;

  // Declare the supported options.
  po::options_description desc("Application options");
  desc.add_options()
      ("help", "Produce help message")
      ("app_cfg",
       po::value(&app_cfg_path)->default_value(app_cfg_path),
       "Main application config file");

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
  // Set up the world, now that params are loaded

  world_builder::World world = world_builder::World(params);

  //////////////////////////////////////////////////////
  // Build the world!

  // Once the generation algorithm is dialed in a bit more, we won't need all
  // these calls to Paint_terrain(); they are there to support the review of
  // the process
  world.Seed_continents();
  world.Paint_terrain();
  writer.Write(world.Get_world_tiles(), params, "1_continents.html");
  world.Seed_oceans();
  world.Paint_terrain();
  writer.Write(world.Get_world_tiles(), params, "2_oceans.html");
  world.Run_diffusion();
  world.Paint_terrain();
  writer.Write(world.Get_world_tiles(), params, "3_tiles_diffused.html");
  world.Normalize_elevation();
  world.Paint_terrain();
  writer.Write(world.Get_world_tiles(), params, "4_height_normalized.html");
  world.Run_oceans_and_coasts();
  world.Paint_terrain();
  writer.Write(world.Get_world_tiles(), params, "5_specify_coasts.html");
  world.Run_rivers();
  world.Paint_terrain();
  writer.Write(world.Get_world_tiles(), params, "6_run_rivers.html");


  //////////////////////////////////////////////////////
  // Output summary

  Print_output_summary(world, params);

  //////////////////////////////////////////////////////
  // Write final world out to HTML

  writer.Write(world.Get_world_tiles(), params, "final_world.html");

  return 0;
}
