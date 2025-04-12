add_library(xdir STATIC
    xdir.cpp
)

target_include_directories(xdir PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)