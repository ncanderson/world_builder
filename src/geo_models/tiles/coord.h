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

};
}

#endif
