cmake_minimum_required(VERSION 3.10)

project(LibCLogger C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_library(logger SHARED logger.c)

target_link_libraries(logger PRIVATE dl)

add_executable(test test.c)

add_executable(reader reader.c)

add_custom_target(run
    COMMAND env "LD_PRELOAD=$<TARGET_FILE:logger>" $<TARGET_FILE:test>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running test with LD_PRELOAD..."
    VERBATIM
)

add_dependencies(run test)