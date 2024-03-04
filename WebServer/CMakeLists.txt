cmake_minimum_required(VERSION 3.0)
project(WebServer)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(main ${SRC_LIST})

target_link_libraries(main pthread)