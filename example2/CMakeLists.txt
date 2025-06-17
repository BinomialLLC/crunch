add_executable (crn_example2
	"example2.cpp"
	"timer.cpp"
	"timer.h"
)
target_link_libraries(crn_example2 crnlib)
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET crn_example2 PROPERTY CXX_STANDARD 20)
endif()