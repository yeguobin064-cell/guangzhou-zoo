cmake_minimum_required(VERSION 3.16)
project(GuangzhouZooRoutePlanner VERSION 1.0.0 LANGUAGES C)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 源文件列表
set(SOURCES
    src/main.c
    src/graph.c
    src/consoleio.c
    src/zoo_data.c
    src/pathfinding.c
)

# 头文件目录
include_directories(include)

# 添加可执行文件
add_executable(zoo_route ${SOURCES})

# 链接数学库（如果需要）
if(NOT MSVC)
    target_link_libraries(zoo_route m)
endif()

# 设置编译选项
if(MSVC)
    target_compile_options(zoo_route PRIVATE /W4 /utf-8)
else()
    target_compile_options(zoo_route PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 复制数据文件到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# 安装规则
install(TARGETS zoo_route DESTINATION bin)
install(DIRECTORY data DESTINATION share/zoo_route)

# 打印配置信息
message(STATUS "================================")
message(STATUS "  广州动物园路线规划系统")
message(STATUS "  版本: ${PROJECT_VERSION}")
message(STATUS "  编译器: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "================================")