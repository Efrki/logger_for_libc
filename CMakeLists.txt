cmake_minimum_required(VERSION 3.10)
project(logger_for_libc C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(logger SHARED logger.c)
target_link_libraries(logger dl)
target_include_directories(logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(daemon reader.c)
target_include_directories(daemon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test test.c)

include(CPack)
set(CPACK_PACKAGE_NAME "logger-for-libc")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "you@example.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LD_PRELOAD-based libc call logger")
set(CPACK_PACKAGE_CONTACT "you@example.com")