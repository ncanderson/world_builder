/**
 * Copyright (C) 2024 Nate Anderson - All Rights Reserved
 */

// Application files
#include <defs/dice_rolls.h>

///////////////////////////////////////////////////////////////////////

std::mt19937& world_builder::dice::Get_generator()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return gen;
}

///////////////////////////////////////////////////////////////////////

bool world_builder::dice::Flip_a_coin()
{
  return static_cast<bool>(Make_a_roll<int8_t>(1, 0));
}

///////////////////////////////////////////////////////////////////////

std::array<unsigned char, 3> world_builder::dice::Create_random_color(int min_value,
                                                                      int max_value)
{
  std::array<unsigned char, 3> color{
      static_cast<unsigned char>(Make_a_roll<int>(min_value, max_value)),
      static_cast<unsigned char>(Make_a_roll<int>(min_value, max_value)),
      static_cast<unsigned char>(Make_a_roll<int>(min_value, max_value))
  };
  return color;
}

///////////////////////////////////////////////////////////////////////
