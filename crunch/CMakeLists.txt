add_executable(crunch
	corpus_gen.cpp
	corpus_test.cpp
	crunch.cpp
)

target_include_directories(crunch
  PUBLIC ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(crunch
	PUBLIC crnlib
)

install(TARGETS crunch DESTINATION bin)
