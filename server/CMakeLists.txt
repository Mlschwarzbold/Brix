add_executable(
    servidor src/main.cpp
    src/greeter/server_UDP_greeter.cpp
    )

# Link with common library if it exists
target_link_libraries(servidor PRIVATE common)

# Include directories
target_include_directories(servidor PRIVATE src)
