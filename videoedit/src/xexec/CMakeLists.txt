add_library(xexec STATIC
    xexec.cpp
)

target_include_directories(xexec PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(xexec PRIVATE 
    xlog  # 依赖 xtask 模块
)