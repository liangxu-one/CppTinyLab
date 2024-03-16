cmake_minimum_required(VERSION 3.0)
project(ChatServer)

# 配置编译选项
set(CMAKE_CXX_FKAGS, ${CMAKE_CXX_FKAGS} -g)

# 添加头文件所在路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/client)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

# 设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 加载子目录
add_subdirectory(src)