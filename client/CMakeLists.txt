add_executable(cliente src/main.cpp)

# Link with common library if it exists
target_link_libraries(cliente PRIVATE common)
