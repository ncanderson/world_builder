/**
 * Copyright (C) 2025 Nate Anderson - All Rights Reserved
 */

#ifndef WORLD_BUILDER_UTILS_H
#define WORLD_BUILDER_UTILS_H

// Standard libs
#include <string>
#include <iostream>
#include <array>

// JSON

// Application files

namespace world_builder
{

/**
 * @brief Template for print to cout handling anything convertable to a string
 * @param log_message The message to print
 */
template<typename C>
void Print_to_cout(const C& log_message)
{
  if constexpr (std::is_convertible_v<C, std::string>)
  {
    // For anything you can convert directly to std::string
      std::cout << "Log Message: " << log_message << std::endl;
  }
  else
  {
    // Fallback: stream it directly (works for arithmetic, and any type with operator<<)
      std::cout << "Log Message: " << log_message << std::endl;
  }
}

// TODO: figure out a better way to do this, such that the templated version will
// handle it and we don't need a separate function
/**
 * @brief Print key value pair for strings
 * @param key_name Name of key
 * @param value Value associated with key
 */
void Print_key_value_string(std::string key_name, std::string value);

/**
 * @brief Print key value pair
 * @tparam T The numeric class to print
 * @param key_name Name of key
 * @param value Value associated with key
 */
template <typename T>
void Print_key_value(std::string key_name, T value)
{
  std::cout << "Log Message: " << key_name << ": " << std::to_string(value) << std::endl;
}

/**
 * @brief Generic helper allowing easier mapping of enums to strings
 * @details This helper struct will allow each enum to define a single
 * enum - string lookup and provide the enum/string conversion functions
 * without additional effort.
 * @tparam Enum The enum to map
 */
template <typename Enum_t>
struct Enum_mapping
{
  /**
   * @brief The enumerated value
   */
  Enum_t enum_value;

  /**
   * @brief The string representation of this enum
   */
  std::string_view enum_string;

  /**
   * @brief Constructor
   * @param enum_value The enum
   * @param enum_string The string representation of the enum
   */
  constexpr Enum_mapping(Enum_t enum_value,
                         std::string_view enum_string)
    :
    enum_value(enum_value),
    enum_string(enum_string)
  { }
};

/**
 * @brief A generic enum to string converter
 * @tparam Enum_t The enum to convert
 * @tparam S The size of the enum to convert (required for the array)
 * @param value The enumerated value to convert
 * @param table The enum to string mapping
 * @return A string_view of this enum.
 * @note The returned string_view is safe as long as the enum - string
 * mapping is defined somewhere. If those strings are not defined (like
 * loaded dynamically from file), these returned string_views may result
 * in dangling references.
 */
template <typename Enum_t, size_t S>
constexpr std::string_view Enum_to_string(Enum_t enum_value,
                                          const std::array<Enum_mapping<Enum_t>, S>& table)
{
  for(auto& enumerated : table)
  {
    if(enumerated.enum_value == enum_value)
    {
      return enumerated.enum_string;
    }
  }
  throw std::invalid_argument("Unknown enum value");
}

/**
 * @brief A generic string to enum converter
 * @tparam Enum_t The enum to get
 * @tparam S The size of the enum to convert (required for the array)
 * @param string The string to convert
 * @param table The enum to string mapping
 * @note Using a string_view is safe as long as the enum - string
 * mapping is defined somewhere. If those strings are not defined (like
 * loaded dynamically from file), these returned string_views may result
 * in dangling references.
 */
template <typename Enum_t, size_t S>
constexpr Enum_t String_to_enum(std::string_view enum_string,
                                const std::array<Enum_mapping<Enum_t>, S>& table)
{
  for(auto& enumerated : table)
  {
    if(enumerated.enum_string == enum_string)
    {
      return enumerated.enum_value;
    }
  }
  throw std::invalid_argument("Unknown enum string");
}

}

#endif
