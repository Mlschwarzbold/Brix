add_executable(
    cliente src/main.cpp
    src/client_UDP_broadcast.cpp
)

# Link with common library
target_link_libraries(cliente PRIVATE common)

# Include directories
target_include_directories(cliente PRIVATE src)
