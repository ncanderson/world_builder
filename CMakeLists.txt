cmake_minimum_required(VERSION 3.5)

set(project_name world_builder)
project(${project_name})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "World builder")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

######################################
# Locations of files for this project

# Source files
file(GLOB_RECURSE glob_headers src/[a-z]*.h)
file(GLOB_RECURSE glob_sources src/[a-z]*.cpp)

# Dependency headers
file(GLOB_RECURSE glob_deps deps/[a-z]*.hpp)

# Collect all sources and headers
set(sources ${glob_sources})
set(headers ${glob_headers} ${glob_deps})

######################################
# Define the main binary

add_executable(${project_name} ${sources} ${headers})

######################################
# Include directories

# Project headers
set(include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/src ${include_dirs})
target_include_directories(${project_name} PRIVATE ${include_dirs})

# Find Boost (for program_options and other headers)
find_package(Boost COMPONENTS program_options REQUIRED)
if(Boost_FOUND)
  target_include_directories(${project_name} PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(${project_name} PRIVATE ${Boost_LIBRARIES})
endif()

######################################
# Preprocessor definitions

# Write output relative to source dir
add_definitions(-DPROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
