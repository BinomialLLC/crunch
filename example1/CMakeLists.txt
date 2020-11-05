add_executable(example1
	example1.cpp
)

target_compile_definitions(example1
	PUBLIC _CRT_SECURE_NO_DEPRECATE
)

target_include_directories(example1
  PUBLIC ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(example1
	PRIVATE crnlib
)

install(TARGETS example1 DESTINATION bin)
