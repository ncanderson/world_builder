/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef HTML_WRITER_H
#define HTML_WRITER_H

// Standard libs
#include <filesystem>

// JSON

// Application files
#include <geo_models/tile.h>
#include <utils/params.h>

namespace world_builder
{
/**
 * @brief The HTML_writer class
 */
class HTML_writer
{
public:
  // Attributes

  // Implementation
  /**
   * @brief Construct a new HTML writer object
   * @param output_dir Directory where the file will be written
   * @param filename Filename (e.g. "index.html")
   */
  explicit HTML_writer(std::filesystem::path output_dir);

  /**
   * @brief Write the world HTML visualization
   * @param world_tiles World tiles to visualize
   * @param params World generation parameters
   * @param filename File to write to
   */
  void Write(const World_tiles& world_tiles,
             const world_builder::Params& params,
             std::string filename = "index.html") const;

private:
  // Attributes
  /**
   * @brief Directory to put html files into
   */
  std::filesystem::path m_output_dir;

  // Implementation

};
}

#endif
