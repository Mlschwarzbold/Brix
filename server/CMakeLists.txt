add_executable(servidor src/main.cpp)

# Link with common library if it exists
target_link_libraries(servidor PRIVATE common)
