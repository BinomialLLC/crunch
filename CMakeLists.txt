# Ensure minimum CMake version.
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# Name the project.
project(crunch-project)

# Add the source library files.
add_subdirectory(crnlib)

# Add the command-line tool.
add_subdirectory(crunch)

# Add the emscripten library.
add_subdirectory(emscripten)

# Add the examples.
add_subdirectory(example1)
add_subdirectory(example2)
add_subdirectory(example3)

install(
	FILES
		${CMAKE_SOURCE_DIR}/inc/crn_decomp.h
		${CMAKE_SOURCE_DIR}/inc/crnlib.h
		${CMAKE_SOURCE_DIR}/inc/dds_defs.h
	DESTINATION
		include
)
