# 定义 xinput 库
add_library(xinput STATIC
    user_input.cpp
    xvideo_input.cpp
)

# 声明依赖：xinput 需要 xtask 的头文件和链接库
target_include_directories(xinput PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(xinput PRIVATE 
    xtask xdir # 依赖 xtask 模块
)

# 可选：如果 xinput 还依赖其他模块（如 xlog）
# target_link_libraries(xinput PRIVATE xlog)