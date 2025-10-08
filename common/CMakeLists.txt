add_library(common
    src/date_time_utils.cpp
    src/packets/packets.cpp
    src/packets/string_packets.cpp
    src/data_transfer/socket_utils.cpp
)

target_include_directories(common PUBLIC src)



#target_include_directories(common
#    PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/src
#)