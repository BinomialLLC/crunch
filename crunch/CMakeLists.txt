add_executable(crunch
    corpus_gen.h
    corpus_gen.cpp
    corpus_test.cpp
    corpus_test.h
    crunch.cpp
)

target_compile_options(crunch
    PRIVATE
        "/W4"           # Warning level 4.
        "/GR-"          # Disable RunRTTI.
)

target_include_directories(crunch
    PUBLIC
        ${CMAKE_SOURCE_DIR}/inc
        ${CMAKE_SOURCE_DIR}/crnlib
)

target_link_libraries(crunch
    crnlib
)

install(TARGETS crunch DESTINATION bin)
