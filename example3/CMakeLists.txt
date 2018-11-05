add_executable(example3
	example3.cpp
)

target_compile_definitions(example3
	PUBLIC _CRT_SECURE_NO_DEPRECATE
)

target_include_directories(example3
  PUBLIC ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(example3
	PRIVATE crnlib
)

install(TARGETS example3 DESTINATION bin)
