add_library(common
    src/date_time_utils.cpp
    src/packets/packets.cpp
)

target_include_directories(common PUBLIC src)



#target_include_directories(common
#    PUBLIC
#        ${CMAKE_CURRENT_SOURCE_DIR}/src
#)