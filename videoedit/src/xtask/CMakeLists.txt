# 定义 xtask 库
add_library(xtask STATIC
    fftask.cpp
    xtask_factory.cpp
)

# 公开头文件路径（让依赖此模块的其他模块自动包含头文件）
target_include_directories(xtask PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}  # 暴露当前目录的头文件
)

target_link_libraries(xtask PRIVATE 
    xexec xlog # 依赖 xlog 模块
)

# 可选：添加 xtask 的私有依赖（如第三方库）
# target_link_libraries(xtask PRIVATE some_external_lib)