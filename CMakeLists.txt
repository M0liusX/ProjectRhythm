cmake_minimum_required(VERSION 2.8)
project("ProjectRhythm")

#set(CMAKE_CXX_STANDARD 14)

# libsoundio options
set(BUILD_EXAMPLE_PROGRAMS OFF CACHE BOOL "Build example programs")
set(BUILD_TESTS OFF CACHE BOOL "Build tests")

add_subdirectory(src)
add_subdirectory(lib/libsoundio)

# Executable Stuff
add_executable("${CMAKE_PROJECT_NAME}" WIN32 ${SOURCES})   # ${ASSETS})

# I am not sure
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libsoundio/
)

get_directory_property(LIBSOUNDIO_LIBS DIRECTORY lib/libsoundio DEFINITION LIBSOUNDIO_LIBS)

target_link_libraries("${CMAKE_PROJECT_NAME}"
    libsoundio_static
    ${LIBSOUNDIO_LIBS}
)