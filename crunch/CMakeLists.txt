add_executable(crunch
	"corpus_gen.cpp"
	"corpus_gen.h"
	"corpus_test.cpp"
	"corpus_test.h"
	"crunch.cpp" 
)

target_link_libraries(crunch crnlib)