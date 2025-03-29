cmake_minimum_required(VERSION 3.10)
project(xlog_decode VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 在Windows上定义特殊的宏
if(WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN -DNOGDI -DNOUSER)
endif()

# 包含目录
include_directories(include)

# 文件工具库
add_library(file_utils STATIC
    src/file_utils.cpp
)

# 文件工具测试程序
add_executable(test_file_utils
    test/test_file_utils_main.cpp
)
target_link_libraries(test_file_utils file_utils)

# 暂时注释掉主程序，因为XlogDecoder类尚未实现
# add_executable(xlog_decoder
#     src/main.cpp
# )
# target_link_libraries(xlog_decoder file_utils)