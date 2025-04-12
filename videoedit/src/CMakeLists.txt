cmake_minimum_required(VERSION 3.10)
project(XVideoEdit)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 设置输出目录（库文件到 lib/，可执行文件到 bin/）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib)  # 静态库 .a/.lib
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib)  # 动态库 .so/.dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)  # 可执行文件

# 根据不同的编译模式设置不同的编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug 模式，启用调试信息
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release 模式，启用优化
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# 添加所有模块
add_subdirectory(xtask)   # xtask 模块
add_subdirectory(xlog)    # xlog 模块
add_subdirectory(xinput)  # xinput 模块（依赖 xtask）
add_subdirectory(xdir)  
add_subdirectory(xexec)  

# 主测试程序
add_executable(test_executable test/test.cpp)
target_link_libraries(test_executable PRIVATE xinput xlog xtask xdir xexec)
