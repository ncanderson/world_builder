/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs

// JSON

// Application files
#include <utils/html_writer.h>

///////////////////////////////////////////////////////////////////////

using html = world_builder::HTML_writer;

///////////////////////////////////////////////////////////////////////

html::HTML_writer(std::filesystem::path output_dir)
  :
  m_output_dir(std::move(output_dir))
{ }

///////////////////////////////////////////////////////////////////////

// TODO Add for loop for the legend, so it will include all enumerated values!
void html::Write(const World_tiles& tiles,
                 const world_builder::Tiles_config& params,
                 std::string filename) const
{
  try
  {
    std::filesystem::create_directories(m_output_dir);
    std::filesystem::path output_file = m_output_dir / filename;
    std::ofstream html(output_file);
    if(!html)
    {
      throw std::runtime_error("Failed to open output HTML file for writing");
    }

    int tile_width = params.Get_width();
    int tile_height = params.Get_height();

    html << R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>)" << filename << R"(</title>
<style>
  html, body { margin:0; padding:0; background:#111; height:100%; width:100%; overflow:hidden; display:flex; justify-content:center; align-items:center; }
  canvas { image-rendering: pixelated; display:block; border:1px solid #333; }
  #legend { position:absolute; top:10px; left:10px; font-family:monospace; color:#eee; background: rgba(0,0,0,0.5); padding:6px 10px; border-radius:6px; }
  .swatch { display:inline-block; width:12px; height:12px; margin-right:6px; vertical-align:middle; }
</style>
</head>
<body>
<div id="legend">
  <div><span class="swatch" style="background:#004;"></span>Ocean</div>
  <div><span class="swatch" style="background:#66f;"></span>River</div>
  <div><span class="swatch" style="background:#eeddaa;"></span>Beach</div>
  <div><span class="swatch" style="background:#88aa55;"></span>Plains</div>
  <div><span class="swatch" style="background:#557744;"></span>Hills</div>
  <div><span class="swatch" style="background:#999;"></span>Mountains</div>
</div>
<canvas id="map"></canvas>
<script>
window.onload = function() {
    const canvas = document.getElementById('map');
    const ctx = canvas.getContext('2d');

    const tileWidth = )" << tile_width << R"(;
    const tileHeight = )" << tile_height << R"(;

    // Tile data
    const tiles = [)";

    // TILE DATA LOOP
    for (int r = 0; r < tile_height; ++r)
    {
      for (int q = 0; q < tile_width; ++q)
      {
        const world_builder::Tile& t = tiles.at({q, r});
        std::string terrain_string;
        terrain_string = std::string(world_builder::Enum_to_string<world_builder::ETerrain>(t.Get_terrain(),
                                                                                            world_builder::TERRAIN_LOOKUP));
        html << "{e:" << t.Get_elevation() << ",t:'" << terrain_string << "'},";
      }
      html << "\n";
    }

    // JAVASCRIPT LOGIC
    html << R"(];

    function drawTiles(scale) {
        ctx.clearRect(0,0,canvas.width,canvas.height);
        for(let y=0; y<tileHeight; y++){
            for(let x=0; x<tileWidth; x++){
                const tile = tiles[y*tileWidth + x];
                let color = '#333';
                switch(tile.t){
                    case 'Ocean': color='#004'; break;
                    case 'River': color='#66f'; break;
                    case 'Beach': color='#eeddaa'; break;
                    case 'Plains': color='#88aa55'; break;
                    case 'Hills': color='#557744'; break;
                    case 'Mountain': color='#999'; break;
                    default: color='#333'; break;
                }
                ctx.fillStyle = color;
                ctx.fillRect(x*scale, y*scale, scale, scale);
            }
        }
    }

    function resizeCanvas() {
        const scaleX = window.innerWidth / tileWidth;
        const scaleY = window.innerHeight / tileHeight;
        const scale = Math.floor(Math.min(scaleX, scaleY));
        canvas.width = tileWidth * scale;
        canvas.height = tileHeight * scale;
        drawTiles(scale);
    }

    window.addEventListener('resize', resizeCanvas);
    resizeCanvas();
};
</script>
</body>
</html>)";

    html.close();
    world_builder::Print_to_cout("HTML world map written to " +
                                 std::string(output_file) + "\n");
  }
  catch (const std::exception& e)
  {
    world_builder::Print_to_cout("WorldHtmlWriter Exception: " +
                                 std::string(e.what()) + "\n");
  }
}

///////////////////////////////////////////////////////////////////////
