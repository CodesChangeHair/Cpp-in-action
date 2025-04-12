add_library(xlog STATIC
    log_console_output.cpp
    log_file_output.cpp
    xlog_format.cpp 
    xconfig.cpp 
    logger.cpp
    log_factory.cpp 

)

target_include_directories(xlog PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)