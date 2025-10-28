/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef COORD_H
#define COORD_H

// Standard libs
#include <cstdint>

// JSON

// Application files

namespace world_builder
{
/**
 * @brief The Coord class
 */
class Coord
{
public:
  // Attributes

  // Implementation
  /**
   * @brief Constructor
   * @param q
   * @param r
   */
  Coord(int32_t q, int32_t r);

  /**
   * @brief Equality overload, for determining if 2 coordinate pairs are the same
   * @param other The other coordinate to check
   * @return True if the coordinates have the same value, otherwise false
   */
  bool operator==(const Coord& other) const noexcept;

  /**
   * Getters and setters
   */
  const int32_t Get_q_coord() const { return m_q_coord; }
  const int32_t Get_r_coord() const { return m_r_coord; }

private:
  /**
   * @brief q coordinate
   */
  int32_t m_q_coord;

  /**
   * @brief r coordinate
   */
  int32_t m_r_coord;

};  // Coord

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
}

#endif
