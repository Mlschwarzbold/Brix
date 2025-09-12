cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Add subdirectories
add_subdirectory(common)   # if you have shared code
add_subdirectory(client)
add_subdirectory(server)