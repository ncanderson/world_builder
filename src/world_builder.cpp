/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <string>

// JSON

// Application files
#include <defs/dice_rolls.h>
#include <utils/tiles_config.h>
#include <utils/world_builder_utils.h>
#include <utils/stopwatch.h>
// Voronoi
#include <utils/voronoi_config.h>
#include <geo_models/voronoi/poisson_disc.h>
#include <geo_models/voronoi/voronoi_builder.h>

///////////////////////////////////////////////////////////////////////

/**
 * @brief Generation type
 */
enum class EGen_type : uint8_t
{
  EGEN_TYPE_Unknown,  ///< Default
  EGEN_TYPE_Tiles,    ///< Tiles
  EGEN_TYPE_Voronoi,  ///< Voronoi grids
  EGEN_TYPE_Count     ///< Size of options enum
};

///////////////////////////////////////////////////////////////////////

/**
 * @brief Parse the argued generation algorithm into an enumerated
 * representation
 * @param gen_type
 * @return The enumerated generation type
 */
EGen_type Parse_gen_type(const std::string& gen_type)
{
  if(gen_type == "tiles")
  {
    return EGen_type::EGEN_TYPE_Tiles;
  }
  else if(gen_type == "voronoi")
  {
    return EGen_type::EGEN_TYPE_Voronoi;
  }
  else
  {
    throw std::invalid_argument("Invalid generation type");
  }
}

///////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  //////////////////////////////////////////////////////
  // Config defaults
  std::string app_cfg_path;

  //////////////////////////////////////////////////////
  // Set up Runtime Objects

  world_builder::Tiles_config tiles_config;
  world_builder::Voronoi_config voronoi_config;

  world_builder::Stopwatch total_timer;
  total_timer.Start();

  std::string gen_type_string;
  EGen_type gen_type = EGen_type::EGEN_TYPE_Unknown;

  //////////////////////////////////////////////////////
  // Set up the program options
  namespace po = boost::program_options;

  // Declare the supported options.
  po::options_description desc("Application options");
  desc.add_options()
      ("help", "Produce help message")
      ("app_cfg",
         po::value(&app_cfg_path)->default_value(app_cfg_path),
         "Main application config file")
      ("gen_type",
         po::value(&gen_type_string),
         "World generation algorithm");


  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  //////////////////////////////////////////////////////
  // Parse options

  if(vm.count("help"))
  {
    world_builder::Print_to_cout(desc);
    return 1;
  }

  if(vm.count("gen_type"))
  {
    gen_type = Parse_gen_type(gen_type_string);
  }
  else
  {
    // Error, exit
    world_builder::Print_to_cout("No generation algorithm specified, exiting");
    return 1;
  }

  if(vm.count("app_cfg"))
  {
    try
    {
      switch(gen_type)
      {
        case(EGen_type::EGEN_TYPE_Tiles):
        {
          tiles_config = world_builder::Tiles_config(std::ifstream(app_cfg_path));
          break;
        }
        case(EGen_type::EGEN_TYPE_Voronoi):
        {
          voronoi_config = world_builder::Voronoi_config(std::ifstream(app_cfg_path));
          break;
        }
        default:
          world_builder::Print_to_cout("No generation algorithm specified, exiting");
          break;
      }
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
  // Build the world

  // Now that I'm doing the config like I am, can re-add the tiles world setup

  // Instantiate the generator
  world_builder::Poisson_disc point_sampler(voronoi_config.Get_width(),
                                            voronoi_config.Get_height(),
                                            voronoi_config.Get_min_distance(),
                                            voronoi_config.Get_attempts());
  // Generate points
  std::vector<world_builder::Point> points = point_sampler.Generate();

  // Output Poisson disc points
  point_sampler.Save_points_as_ppm("/home/nanderson/nate_personal/projects/world_builder/output/poisson_points.ppm");

  //////////////////////////////////////////////////////
  // Points to Voronoi polygons

  world_builder::Voronoi_builder voronoi_builder(voronoi_config.Get_width(),
                                                voronoi_config.Get_height(),
                                                voronoi_config.Get_voronoi_scale_factor());

  std::vector<world_builder::Cell> cells = voronoi_builder.Build_cells(points);

  // Show colored cells
  voronoi_builder.Export_PPM("/home/nanderson/nate_personal/projects/world_builder/output/voronoi_cells.ppm");

  //////////////////////////////////////////////////////
  // World Visualization


  return 0;
}
