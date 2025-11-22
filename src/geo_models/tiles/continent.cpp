/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

// Standard libs

// JSON

// Application files
#include <geo_models/tiles/continent.h>

///////////////////////////////////////////////////////////////////////

using cont = world_builder::Continent;

///////////////////////////////////////////////////////////////////////

cont::Continent(int32_t center_q, int32_t center_r, double radius)
  :
  m_center_q(center_q),
  m_center_r(center_r),
  m_radius(radius)
{ }

///////////////////////////////////////////////////////////////////////

