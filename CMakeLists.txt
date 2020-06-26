cmake_minimum_required(VERSION 2.8.12.2)

project (crunch)
set( CMAKE_CXX_FLAGS "-Wextra -Wall -fomit-frame-pointer -fexpensive-optimizations -O3 -fexceptions -Wno-unused-value -Wno-unused -fno-strict-aliasing -ffast-math -fno-math-errno " )

add_subdirectory (crnlib)

include_directories(inc)

add_executable ( crunch
		crunch/corpus_gen.cpp
		crunch/corpus_gen.h
		crunch/corpus_test.cpp
		crunch/corpus_test.h
		crunch/crunch.cpp
)

# Link the executable to the crn library. Since the crn library has
# public include directories we will use those link directories when building
# crunch
target_link_libraries (crunch LINK_PUBLIC crn pthread)

install (TARGETS crunch DESTINATION bin) # installs crunch to /usr/local/bin/crunch
