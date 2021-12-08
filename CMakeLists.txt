# cmake 执行命令(在项目根目录下): cmake -B build
cmake_minimum_required (VERSION 3.2) 

project (smuduo) 

# 设置C++编译选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")

# 添加宏的定义
add_definitions(-D USE_LOG_DEBUG)
add_definitions(-D USE_POLL)

# 设置可执行文件和库文件的输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置头文件和库搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${LIBRARY_OUTPUT_PATH})

# 进入子文件夹
add_subdirectory(src)
add_subdirectory(test)

message(STATUS "the BINARY dir is ${PROJECT_BINARY_DIR}")
message(STATUS "the SOURCE dir is ${PROJECT_SOURCE_DIR}")