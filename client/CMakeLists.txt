add_executable(
    cliente src/main.cpp
    src/client_discovery_protocol.cpp
    src/data_transfer/client_request_transfer.cpp
)

# Link with common library
target_link_libraries(cliente PRIVATE common)

# Include directories
target_include_directories(cliente PRIVATE src)
