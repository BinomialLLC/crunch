add_library(emscripten STATIC
	crunch_lib.cpp
)

target_include_directories(emscripten
  PUBLIC ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(emscripten
	PRIVATE crnlib
)

install(TARGETS emscripten DESTINATION lib)
