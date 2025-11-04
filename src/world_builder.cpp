/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */


// Standard libs
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

// JSON

// Application files
#include <defs/dice_rolls.h>
#include <utils/params.h>
#include <utils/world_builder_utils.h>
#include <utils/stopwatch.h>

///////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  //////////////////////////////////////////////////////
  // Config defaults
  std::string app_cfg_path = "/home/nanderson/nate_personal/projects/world_builder/config/gen_params.json";

  //////////////////////////////////////////////////////
  // Set up Runtime Objects

  world_builder::Params params;
  world_builder::Stopwatch total_timer;
  total_timer.Start();

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
  // Set up the world

  //////////////////////////////////////////////////////
  // Build the world

  //////////////////////////////////////////////////////
  // World Visualization

  return 0;
}
