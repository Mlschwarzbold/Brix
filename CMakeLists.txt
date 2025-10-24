cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

add_compile_options(-Wall -Werror)
add_compile_options("-O0")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_CXX_STANDARD 17)

# Add subdirectories
add_subdirectory(common)   
add_subdirectory(client)
add_subdirectory(server)