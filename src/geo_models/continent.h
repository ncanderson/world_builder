/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef CONTINENT_H
#define CONTINENT_H

// Standard libs
#include <cstdint>

// JSON

// Application files

namespace world_builder
{
/**
 * @brief The Continent class
 */
class Continent
{
public:
  // Attributes

  // Implementation
  Continent(int32_t center_q, int32_t center_r, double radius);

  /**
   * Getters and setters
   */
  const int32_t Get_center_q() const { return m_center_q; }
  const int32_t Get_center_r() const { return m_center_r; }
  const double Get_radius() const { return m_radius; }

private:
  // Attributes
  /**
   * @brief Q-coordinate of the center of the continent
   */
  int32_t m_center_q;

  /**
   * @brief R-coordinate of the center of the continent
   */
  int32_t m_center_r;

  /**
   * @brief Radius of the continent
   */
  double m_radius;

  // Implementation
};
}

#endif
