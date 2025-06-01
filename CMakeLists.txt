cmake_minimum_required(VERSION 3.10)
project(End2End)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "g++")

# 使用O3优化
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# 优先查找静态库
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

# 头文件
include_directories(/usr/include) # GMP头文件

# 创建目录
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/test)

# 源文件
file(GLOB_RECURSE GETPRIME "getPrime/getPrime.cpp")
file(GLOB_RECURSE ELGAMAL "elgamal/elgamal.cpp")
file(GLOB_RECURSE SM4 "sm4/sm34.cpp")
file(GLOB_RECURSE ENCRYPTER "encrypter/encrypter.cpp")
file(GLOB_RECURSE FRONTEND "frontend/web.cpp")

# 创建链接库 
add_library(core_lib STATIC
    ${GETPRIME}
    ${SM4}
    ${ELGAMAL}
    ${ENCRYPTER}
    core/core.cpp
)

# 设置链接库属性
set_target_properties(core_lib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(core_lib gmp gmpxx pthread)

# 添加可执行文件
add_executable(test_client core/test_client.cpp)
add_executable(test_server core/test_server.cpp)

add_executable(end2end ${FRONTEND} main.cpp)

# 处理静态链接
function(configure_target target_name output_dir)
    # 设置输出目录
    set_target_properties(${target_name} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${output_dir})
    
    # 设置静态链接属性
    set_target_properties(${target_name} PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties(${target_name} PROPERTIES LINK_SEARCH_END_STATIC ON)
    
    # 链接库
    target_link_libraries(${target_name} -static core_lib)
endfunction()

# 配置目标
configure_target(test_client ${PROJECT_SOURCE_DIR}/test)
configure_target(test_server ${PROJECT_SOURCE_DIR}/test)
configure_target(end2end ${PROJECT_SOURCE_DIR})

# make clean-all 
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/test
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_SOURCE_DIR}/web_server
    COMMENT "Cleaning all build artifacts"
)

