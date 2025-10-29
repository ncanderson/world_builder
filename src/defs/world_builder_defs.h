/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef WORLD_BUILDER_DEFS_H
#define WORLD_BUILDER_DEFS_H

// Standard libs
#include <cstdlib>

// JSON

// Application files

namespace world_builder
{

/**
 * Forward declarations to avoid including these classes into this header,
 * which will prevent circular includes
 */
class Coord;

/**
 * @brief Custom hash for unordered_map, ensuring no duplicate coordinates
 */
struct Coord_hash
{
  /**
   * @brief Hashing functor (ie an object that can be called like a function)
   * @details
   * - `operator()`: This the 'function call operator', and allows an instance
   *                 of Coord_hash to be called like a function; note that
   *                 `std::hash<int>()` is itself a functor.
   * - `const`: Hash functions should be pure (no side effects), so `const` is standard
   * - `noexcept`: This cannot throw exceptions
   * - `std::hash<int>()()`: Uses an inline functor to hash the q coordinate,
   *                         then shifts 1 bit to the left, used to spread out bits
   *                         to reduce collisions
   * - `^`: Bitwise XOR, which mooshes together the two coords.
   * @param coord The coordinate to hash
   * @return The hashed coordinates
   */
  std::size_t operator()(const Coord& coord) const noexcept;

}; // Coord_hash
}  // namespace world_builder

#endif
