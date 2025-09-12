add_library(common
    src/date_time_utils.cpp
)

target_include_directories(common PUBLIC include)


# Tell consumers of common where to find headers
target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)