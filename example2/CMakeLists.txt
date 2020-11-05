add_executable(example2
	example2.cpp
	timer.cpp
)

target_compile_definitions(example2
	PUBLIC _CRT_SECURE_NO_DEPRECATE
)

target_include_directories(example2
  PUBLIC ${CMAKE_SOURCE_DIR}/inc
)

target_link_libraries(example2
	PRIVATE crnlib
)

install(TARGETS example2 DESTINATION bin)
