add_executable(crn_example1
	"example1.cpp"
	"stb_image.h"
)
target_link_libraries(crn_example1 crnlib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET crn_example1 PROPERTY CXX_STANDARD 20)
endif()
