#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <cmath>
#include <algorithm>
#include <tuple>
#include <string>

struct Coord {
    int q;
    int r;
    bool operator==(const Coord& other) const noexcept {
        return q == other.q && r == other.r;
    }
};

// Custom hash for unordered_map
struct CoordHash {
    std::size_t operator()(const Coord& c) const noexcept {
        return (std::hash<int>()(c.q) << 1) ^ std::hash<int>()(c.r);
    }
};

struct Tile {
    Coord coord;
    double elevation = 0.0;
    std::string terrain = "unknown";
    bool isRiver = false;
    bool isCoast = false;
    std::optional<Coord> riverTo;
};

// axial hex neighbor offsets
const std::vector<Coord> neighbor_offsets = {
    {+1, 0}, {+1, -1}, {0, -1}, {-1, 0}, {-1, +1}, {0, +1}
};

std::vector<Coord> neighbors(const Coord& c) {
    std::vector<Coord> n;
    for (auto& o : neighbor_offsets) {
        n.push_back({c.q + o.q, c.r + o.r});
    }
    return n;
}

// Parameters
struct Params {
    int width = 80;
    int height = 48;
    int smooth_passes = 6;
    double randomness = 0.9;
    double sea_level = 0.15;
    double river_spawn_prob = 0.02;
    int max_river_length = 300;
    unsigned seed = 42;
};

// Simple RNG
struct RNG {
    std::mt19937 gen;
    RNG(unsigned seed) : gen(seed) {}
    double uniform(double a=0.0, double b=1.0) {
        std::uniform_real_distribution<double> dist(a, b);
        return dist(gen);
    }
    int randint(int a, int b) {
        std::uniform_int_distribution<int> dist(a, b);
        return dist(gen);
    }
};

// Downhill neighbor search
std::optional<Coord> downhillNeighbor(
    const Coord& c,
    const std::unordered_map<Coord, Tile, CoordHash>& tiles)
{
    auto it = tiles.find(c);
    if (it == tiles.end()) return std::nullopt;
    double cur_e = it->second.elevation;
    std::vector<std::pair<Coord, double>> cands;
    for (auto& n : neighbors(c)) {
        auto it2 = tiles.find(n);
        if (it2 != tiles.end()) {
            cands.push_back({n, it2->second.elevation});
        }
    }
    if (cands.empty()) return std::nullopt;
    std::sort(cands.begin(), cands.end(),
              [](auto& a, auto& b){ return a.second < b.second; });
    if (cands.front().second <= cur_e)
        return cands.front().first;
    return std::nullopt;
}

// Trace one river
std::vector<Coord> traceRiver(
    const Coord& start,
    std::unordered_map<Coord, Tile, CoordHash>& tiles,
    const Params& P)
{
    std::vector<Coord> path;
    Coord cur = start;
    std::unordered_map<Coord, bool, CoordHash> visited;
    for (int step = 0; step < P.max_river_length; ++step) {
        if (visited[cur]) break;
        visited[cur] = true;
        path.push_back(cur);
        auto& tile = tiles[cur];
        if (tile.elevation <= P.sea_level)
            break; // reached sea
        auto dn = downhillNeighbor(cur, tiles);
        if (!dn) break;
        cur = *dn;
    }
    return path;
}

int main() {
    Params P;
    RNG rng(P.seed);

    std::unordered_map<Coord, Tile, CoordHash> tiles;
    for (int q = 0; q < P.width; ++q) {
        for (int r = 0; r < P.height; ++r) {
            Coord c{q, r};
            tiles[c] = Tile{c};
        }
    }

    // Seed elevations
    int numSeeds = std::max(6, (P.width * P.height) / 400);
    for (int i = 0; i < numSeeds; ++i) {
        int q = rng.randint(0, P.width - 1);
        int r = rng.randint(0, P.height - 1);
        tiles[{q, r}].elevation = rng.uniform(0.0, 1.0);
    }
    for (int i = 0; i < numSeeds; ++i) {
        int q = rng.randint(0, P.width - 1);
        int r = rng.randint(0, P.height - 1);
        tiles[{q, r}].elevation = rng.uniform(-0.5, 0.2);
    }

    // Diffusion / smoothing
    for (int pass = 0; pass < P.smooth_passes; ++pass) {
        std::unordered_map<Coord, double, CoordHash> new_elev;
        for (auto& [coord, tile] : tiles) {
            auto n = neighbors(coord);
            std::vector<double> nvals;
            for (auto& nn : n) {
                auto it2 = tiles.find(nn);
                if (it2 != tiles.end()) nvals.push_back(it2->second.elevation);
            }
            double nbr_mean = nvals.empty() ? tile.elevation :
                                  std::accumulate(nvals.begin(), nvals.end(), 0.0) / nvals.size();
            double blend = 0.6;
            double noise = (rng.uniform() - 0.5) * P.randomness * (1.0 - (double)pass / P.smooth_passes);
            new_elev[coord] = nbr_mean * blend + tile.elevation * (1 - blend) + noise;
        }
        for (auto& [coord, val] : new_elev) {
            tiles[coord].elevation = val;
        }
    }

    // Normalize elevation
    double minE = 1e9, maxE = -1e9;
    for (auto& [c, t] : tiles) {
        minE = std::min(minE, t.elevation);
        maxE = std::max(maxE, t.elevation);
    }
    for (auto& [c, t] : tiles) {
        if (maxE > minE) t.elevation = (t.elevation - minE) / (maxE - minE);
        else t.elevation = 0.0;
    }

    // Terrain classification
    for (auto& [c, t] : tiles) {
        if (t.elevation <= P.sea_level) t.terrain = "ocean";
    }
    // Mark coasts
    for (auto& [c, t] : tiles) {
        if (t.terrain == "ocean") continue;
        for (auto& n : neighbors(c)) {
            auto it2 = tiles.find(n);
            if (it2 != tiles.end() && it2->second.terrain == "ocean") {
                t.isCoast = true;
                break;
            }
        }
    }

    // Rivers
    std::vector<std::vector<Coord>> rivers;
    for (auto& [c, t] : tiles) {
        if (t.elevation > P.sea_level + 0.05 && rng.uniform() < P.river_spawn_prob) {
            auto path = traceRiver(c, tiles, P);
            if (path.size() >= 3) {
                for (size_t i = 0; i + 1 < path.size(); ++i) {
                    tiles[path[i]].isRiver = true;
                    tiles[path[i]].riverTo = path[i + 1];
                }
                tiles[path.back()].isRiver = true;
                rivers.push_back(path);
            }
        }
    }

    // Terrain painting
    for (auto& [c, t] : tiles) {
        if (t.terrain == "ocean") continue;
        if (t.isRiver) { t.terrain = "river"; continue; }
        if (t.isCoast && t.elevation <= P.sea_level + 0.03) {
            t.terrain = "beach";
        } else if (t.elevation < P.sea_level + 0.07) {
            t.terrain = "marsh";
        } else if (t.elevation < P.sea_level + 0.20) {
            t.terrain = "plains";
        } else if (t.elevation < P.sea_level + 0.45) {
            t.terrain = "hills";
        } else {
            t.terrain = (t.elevation > 0.8 ? "mountain" : "hills");
        }
    }

    // Print summary
    std::cout << "World generated (" << P.width << "x" << P.height
              << "), rivers: " << rivers.size() << "\n";
    std::unordered_map<std::string, int> counts;
    for (auto& [c, t] : tiles) counts[t.terrain]++;
    for (auto& [name, n] : counts) {
        std::cout << "  " << name << ": " << n << "\n";
    }

    // Simple ASCII elevation preview
    const std::string chars = " .,:;irsXA253hMHGS#9B&@";
    for (int r = P.height - 1; r >= 0; --r) {
        for (int q = 0; q < P.width; ++q) {
            auto it = tiles.find({q, r});
            if (it == tiles.end()) { std::cout << ' '; continue; }
            const Tile& t = it->second;
            if (t.terrain == "ocean") std::cout << '~';
            else if (t.terrain == "river") std::cout << '/';
            else {
                int idx = std::clamp<int>(int(t.elevation * (chars.size() - 1)), 0, chars.size() - 1);
                std::cout << chars[idx];
            }
        }
        std::cout << "\n";
    }

    return 0;
}
