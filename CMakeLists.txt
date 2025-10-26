cmake_minimum_required(VERSION 3.16)
project(BRIX CXX)

add_compile_options(-Wall -Werror)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_STANDARD 17)

# Define DEBUG macro for Debug builds
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

# Add subdirectories
add_subdirectory(common)   
add_subdirectory(client)
add_subdirectory(server)

