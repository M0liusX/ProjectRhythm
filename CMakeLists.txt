cmake_minimum_required(VERSION 2.8)
project("ProjectRhythm")
#set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/gainput)
add_subdirectory(src)          # Depends on gainput


# Executable Stuff
add_executable("${CMAKE_PROJECT_NAME}" WIN32 ${SOURCES})

# Libraries
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/soundio/
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/gainput/lib/include
)

# Linking
if(${CMAKE_SYSTEM_NAME} MATCHES WIN32)
    target_link_libraries("${CMAKE_PROJECT_NAME}" ${XINPUT} ws2_32)
endif()