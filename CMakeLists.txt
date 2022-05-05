set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

# 3.7 is required to find vulkan
cmake_minimum_required(VERSION 3.7)

# creating the project
project(undicht)

# setting cpp standard
set (CMAKE_CXX_STANDARD 11)

# adding libraries
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(tools)

# adding examples
add_subdirectory(examples/hello_world)
