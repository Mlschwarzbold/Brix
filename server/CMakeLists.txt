add_executable(
    servidor src/main.cpp
    src/greeter/server_UDP_greeter.cpp
    src/greeter/server_discovery_service.cpp
    src/db_manager/db_manager.cpp
    src/multiplexer/packet_multiplexer.cpp
    src/request_handler/REQ_packet_handler.cpp
    src/request_handler/KIL_packet_handler.cpp
    )

# Link with common library if it exists
target_link_libraries(servidor PRIVATE common)

# Include directories
target_include_directories(servidor PRIVATE src)
