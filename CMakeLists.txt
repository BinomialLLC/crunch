# Ensure minimum CMake version.
cmake_minimum_required(VERSION 3.11.1 FATAL_ERROR)

# Name the project.
project(Cobra)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Add the library.
add_subdirectory(crnlib)

# Add the command-line tool.
add_subdirectory(crunch)
