# CMakeList.txt : CMake project for CommonDataLibs, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Crunch")


# These are turned off by default.
option(CRUNCH_BUILD_CRUNCH_EXECUTABLE "Build Crunch executable" OFF)
option(CRUNCH_BUILD_CRUNCH_EXAMPLES "Build Crunch Examples" OFF)
# Add source to this project's executable.

include_directories("inc")

add_subdirectory("crnlib")

if(CRUNCH_BUILD_CRUNCH_EXECUTABLE)
	add_subdirectory("crunch")
endif()

#TODO: Test examples on other platforms
if(CRUNCH_BUILD_CRUNCH_EXAMPLES AND WIN32)
	#crunch Example1
	add_subdirectory("example1")

	#crunch Example2:
	add_subdirectory("example2")

	#crunch Example3:
	add_subdirectory("example3")
endif()
# TODO: Add tests and install targets if needed.
