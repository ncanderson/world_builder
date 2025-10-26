#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

struct Coord
{
  int q;
  int r;
  bool operator==(const Coord& other) const noexcept
  {
    return q == other.q && r == other.r;
  }
};

// Custom hash for unordered_map, ensuring no duplicates
struct CoordHash
{
  std::size_t operator()(const Coord& c) const noexcept
  {
    return (std::hash<int>()(c.q) << 1) ^ std::hash<int>()(c.r);
  }
};

struct Tile
{
  Coord coord;
  double elevation = 0.0;
  std::string terrain = "unknown";
  bool isRiver = false;
  bool isCoast = false;
  std::optional<Coord> riverTo;
};

// axial hex neighbor offsets ie making hexex
const std::vector<Coord> neighbor_offsets = {
  {+1, 0}, {+1, -1}, {0, -1}, {-1, 0}, {-1, +1}, {0, +1}
};

std::vector<Coord> get_neighbor_tiles(const Coord& c)
{
  std::vector<Coord> n;
  for (auto& o : neighbor_offsets)
  {
    n.push_back({c.q + o.q, c.r + o.r});
  }
  return n;
}

struct Continent
{
  int centerQ;
  int centerR;
  double radius;
};

// Parameters
struct Params
{
  /**
   * @brief Map width
   */
  int width = 300;

  /**
   * @brief Map height
   */
  int height = 150;

  /**
   * @brief Fewer passes give a rougher map
   */
  int smooth_passes = 6;

  /**
   * @brief Global terrain roughness factor
   * @details Higher values yield more random terrain
   */
  double randomness = 0.8;

  double sea_level = 0.5;

  double river_spawn_prob = 0.02;

  int max_river_length = 300;

  // Random seed, used to generate the rest of the randomness
  unsigned seed = std::random_device{}();;
};

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

// Downhill neighbor search
std::optional<Coord> downhillNeighbor(const Coord& c,
                                      const std::unordered_map<Coord, Tile, CoordHash>& tiles)
{
  auto it = tiles.find(c);
  if(it == tiles.end())
  {
    return std::nullopt;
  }

  double cur_e = it->second.elevation;

  std::vector<std::pair<Coord, double>> cands;

  for(auto& n : get_neighbor_tiles(c))
  {
    auto it2 = tiles.find(n);
    if(it2 != tiles.end())
    {
      cands.push_back({n, it2->second.elevation});
    }
  }

  if(cands.empty())
  {
    return std::nullopt;
  }

  std::sort(cands.begin(), cands.end(),
            [](auto& a, auto& b){ return a.second < b.second; });
  if(cands.front().second <= cur_e)
  {
    return cands.front().first;
  }
  return std::nullopt;
}

// Trace one river
std::vector<Coord> traceRiver(const Coord& start,
                              std::unordered_map<Coord, Tile, CoordHash>& tiles,
                              const Params& P)
{
  std::vector<Coord> path;
  Coord cur = start;
  std::unordered_map<Coord, bool, CoordHash> visited;
  for(int step = 0; step < P.max_river_length; ++step)
  {
    if(visited[cur])
    {
      break;
    }
    visited[cur] = true;
    path.push_back(cur);

    auto& tile = tiles[cur];
    if(tile.elevation <= P.sea_level)
    {
      break; // reached sea
    }
    auto dn = downhillNeighbor(cur, tiles);
    if(!dn)
    {
      break;
    }
    cur = *dn;
  }
  return path;
}

int main()
{
  Params P;
  RNG rng(P.seed);

  std::unordered_map<Coord, Tile, CoordHash> tiles;

  // Using the params, build a grid of Coord objects, which are then used to
  // build a Tile. These tiles represent the individual unit that builds the
  // whole map.
  for (int q = 0; q < P.width; ++q)
  {
    for (int r = 0; r < P.height; ++r)
    {
      Coord c{q, r};
      tiles[c] = Tile{c};
    }
  }

  // ----------------------
  // Continent-based seeding
  // ----------------------

  // Define 2–4 continents depending on map size
  int numContinents = std::max(2, P.width / 40);
  std::vector<Continent> continents;
  for (int i = 0; i < numContinents; ++i)
  {
    continents.push_back({
        rng.randint(P.width / 8, P.width * 7 / 8),    // centerQ
        rng.randint(P.height / 8, P.height * 7 / 8),  // centerR
        rng.uniform(P.width / 6.0, P.width / 4.0)     // radius
    });
  }

  // Place elevation seeds around each continent center
  int seedsPerContinent = std::max(3, (P.width * P.height) / (200 * numContinents));
  for (auto& c : continents)
  {
    for (int i = 0; i < seedsPerContinent; ++i)
    {
      double angle = rng.uniform(0, 2 * M_PI);
      double dist = rng.uniform(0, c.radius);
      int q = c.centerQ + static_cast<int>(dist * cos(angle));
      int r = c.centerR + static_cast<int>(dist * sin(angle));

      if(q >= 0 && q < P.width && r >= 0 && r < P.height)
      {
        // Skewed toward land
        tiles[{q, r}].elevation = rng.uniform(0.4, 1.0);
      }
    }
  }

  // Only add ocean seeds outside continents
  int oceanSeeds = seedsPerContinent; // same count as land seeds
  for (int i = 0; i < oceanSeeds; ++i)
  {
    int q = rng.randint(0, P.width - 1);
    int r = rng.randint(0, P.height - 1);

    // skip tiles that are close to a continent center
    bool nearContinent = false;
    for (auto& c : continents)
    {
      double dq = q - c.centerQ;
      double dr = r - c.centerR;
      double dist = std::sqrt(dq*dq + dr*dr);
      if (dist < c.radius * 0.8)
      {
        nearContinent = true;
        break;
      }
    }

    if(!nearContinent)
    {
      tiles[{q, r}].elevation = rng.uniform(-0.5, 0.2);
    }
  }

  // Diffusion / smoothing. For every smoothing pass, this will set the elevation for each
  // to based on the average of all neighbors with some random noise injected.
  for (int pass = 0; pass < P.smooth_passes; ++pass)
  {
    // Temp map to hold all coordinates, allowing update of the main map after
    // doing the smoothing passes
    std::unordered_map<Coord, double, CoordHash> new_elev;

    // ie for every tile...
    for (auto& [coord, tile] : tiles)
    {
      // get all neighbors for this tile
      auto neighbors = get_neighbor_tiles(coord);

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
          neighbor_elevations.push_back(it2->second.elevation);
        }
      }

      // Average all neighboring elevations
      double nbr_mean = 0;

      // If no neighbors, use this tile's own elevation
      if(neighbor_elevations.empty())
      {
        nbr_mean = tile.elevation;
      }
      // Otherwise, add up all neighboring elevations and divide by found neighbors
      else
      {
        nbr_mean = std::accumulate(neighbor_elevations.begin(),
                                   neighbor_elevations.end(), 0.0) / neighbor_elevations.size();
      }

      double blend = 0.6;

      // (rng.uniform() - 0.5): Make the number in the range of -.5 to .5
      // * P.randomness: Augment the random roll with the additional randomness factor
      // (1.0 - (double)pass / P.smooth_passes): Dampens the noise gradually with each smoothing pass.
      double noise = (rng.uniform() - 0.5) * P.randomness * (1.0 - (double)pass / P.smooth_passes);

      // New elevation is a weighted average between (old elevation) and (neighbor mean), plus some fading noise.
      // If blend = 0.5 → half current height, half neighbors → moderate smoothing.
      // If blend = 1.0 → completely replace with neighbor mean (max smoothing).
      // If blend = 0.0 → do nothing (preserve current map).
      new_elev[coord] = nbr_mean * blend + tile.elevation * (1 - blend) + noise;
    }

    // Reset the tiles to the new elevation
    for (auto& [coord, val] : new_elev)
    {
      tiles[coord].elevation = val;
    }
  }

  // Normalize elevation
  double minE = 1e9;
  double maxE = -1e9;

  // For every tile, check for a new max or min elevation
  for(auto& [c, t] : tiles)
  {
    minE = std::min(minE, t.elevation);
    maxE = std::max(maxE, t.elevation);
  }

  // For every tile, re-scale the elevation to fit within a 0 - 1 range
  for(auto& [c, t] : tiles)
  {
    t.elevation = (t.elevation - minE) / (maxE - minE);
  }

  // Terrain classification based on elevation
  for(auto& [c, t] : tiles)
  {
    if(t.elevation <= P.sea_level)
    {
      t.terrain = "ocean";
    }
  }

  // Mark coasts
  for(auto& [c, t] : tiles)
  {
    // Ignore oceans
    if(t.terrain == "ocean")
    {
      continue;
    }

    // For every tile, if it's not an ocean but a neighbor is an ocean, then
    // this is a coast
    for(auto& n : get_neighbor_tiles(c))
    {
      auto it2 = tiles.find(n);
      if(it2 != tiles.end() && it2->second.terrain == "ocean")
      {
        t.isCoast = true;
        break;
      }
    }
  }

  // Rivers
  std::vector<std::vector<Coord>> rivers;
  // for every tile,
  for(auto& [c, t] : tiles)
  {
    // Check elevation. greater than sea level (plus a pad), and make a roll against probability
    if(t.elevation > P.sea_level + 0.05 && rng.uniform() < P.river_spawn_prob)
    {
      // Trace a river path,
      auto path = traceRiver(c, tiles, P);
      // if there are three or more tiles,
      if(path.size() >= 3)
      {
        // for each tile in the river path,
        for(size_t i = 0; i + 1 < path.size(); ++i)
        {
          // set some attributes
          tiles[path[i]].isRiver = true;
          tiles[path[i]].riverTo = path[i + 1];
        }
        tiles[path.back()].isRiver = true;
        rivers.push_back(path);
      }
    }
  }

  // Terrain painting
  for (auto& [c, t] : tiles)
  {
    if (t.terrain == "ocean")
    {
      continue;
    }

    if(t.isRiver)
    {
      t.terrain = "river";
      continue;
    }

    if(t.isCoast && t.elevation <= P.sea_level + 0.03)
    {
        t.terrain = "beach";
    }
    else if(t.elevation < P.sea_level + 0.07)
    {
      t.terrain = "marsh";
    }
    else if(t.elevation < P.sea_level + 0.20)
    {
      t.terrain = "plains";
    }
    else if(t.elevation < P.sea_level + 0.45)
    {
      t.terrain = "hills";
    }
    else
    {
      t.terrain = (t.elevation > 0.8 ? "mountain" : "hills");
    }
  }

  // Print summary
  std::cout << "World generated (" << P.width << "x" << P.height
            << "), rivers: " << rivers.size() << "\n";
  std::unordered_map<std::string, int> counts;

  for(auto& [c, t] : tiles)
  {
    counts[t.terrain]++;
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
  body { margin: 0; background: #111; display: flex; align-items: center; justify-content: center; height: 100vh; }
  canvas { image-rendering: pixelated; border: 1px solid #444; }
  #legend { position: fixed; top: 10px; left: 10px; font-family: monospace; color: #eee; background: rgba(0,0,0,0.5); padding: 6px 10px; border-radius: 6px; }
  .swatch { display: inline-block; width: 12px; height: 12px; margin-right: 6px; vertical-align: middle; }
</style>
</head>
<body>
<div id="legend">
  <div><span class="swatch" style="background:#004;"></span>Ocean</div>
  <div><span class="swatch" style="background:#66f;"></span>River</div>
  <div><span class="swatch" style="background:#eeddaa;"></span>Beach</div>
  <div><span class="swatch" style="background:#88aa55;"></span>Plains</div>
  <div><span class="swatch" style="background:#557744;"></span>Hills</div>
  <div><span class="swatch" style="background:#999999;"></span>Mountains</div>
</div>
<canvas id="map" width=)" << P.width * scale << " height=" << P.height * scale << R"(></canvas>
<script>
const canvas = document.getElementById('map');
const ctx = canvas.getContext('2d');
const scale = )" << scale << R"(;
const tilesWidth = )" << P.width << R"(;
const tilesHeight = )" << P.height << R"(;

// Load tile data
const tiles = [
)";

      for (int r = 0; r < P.height; ++r)
      {
        for (int q = 0; q < P.width; ++q)
        {
          const Tile& t = tiles.at({q, r});
          html << "{e:" << t.elevation << ",t:'" << t.terrain << "'},";
        }
        html << "\n";
      }

      html << R"(];

// Draw each tile as a colored rectangle
for (let y = 0; y < tilesHeight; ++y) {
  for (let x = 0; x < tilesWidth; ++x) {
    const tile = tiles[y * tilesWidth + x];
    let color;
    switch(tile.t) {
      case 'ocean': color = `rgb(0,0,${100 + tile.e*100})`; break;
      case 'river': color = 'rgb(80,120,255)'; break;
      case 'beach': color = 'rgb(238,214,175)'; break;
      case 'marsh': color = 'rgb(110,150,90)'; break;
      case 'plains': color = 'rgb(136,170,85)'; break;
      case 'hills': color = 'rgb(85,119,68)'; break;
      case 'mountain': color = `rgb(${150+tile.e*100},${150+tile.e*100},${150+tile.e*100})`; break;
      default: color = 'rgb(50,50,50)';
    }
    ctx.fillStyle = color;
    ctx.fillRect(x*scale, (tilesHeight-1-y)*scale, scale, scale);
  }
}
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
