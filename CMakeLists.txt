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

# 添加可执行文件
# add_executable(Miller-Rabin ${GETPRIME} getPrime/test_MillerRabbin.cpp)
# add_executable(Miller-Rabbin ${GETPRIME} getPrime/test/Miller-Rabbin.cpp)
# add_executable(test_mod ${GETPRIME} getPrime/test/test_mod.cpp)
# add_executable(ElGamal ${GETPRIME} ${ELGAMAL} elgamal/test_elgamal.cpp)
add_executable(encrypter ${GETPRIME} ${SM4} ${ELGAMAL} ${ENCRYPTER} encrypter/test_encrypter.cpp)
# add_executable(getPrime_test ${GETPRIME} getPrime/test_getPrime.cpp)

# 处理静态链接
function(configure_target target_name output_dir)
    # 设置输出目录
    set_target_properties(${target_name} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${output_dir})
    
    # 设置静态链接属性
    set_target_properties(${target_name} PROPERTIES LINK_SEARCH_START_STATIC ON)
    set_target_properties(${target_name} PROPERTIES LINK_SEARCH_END_STATIC ON)
    
    # 链接库
    target_link_libraries(${target_name} -static gmp gmpxx)
endfunction()

# configure_target(End2End ${PROJECT_SOURCE_DIR})
# configure_target(Miller-Rabin ${PROJECT_SOURCE_DIR}/test)
# configure_target(test_mod ${PROJECT_SOURCE_DIR}/test)
# configure_target(ElGamal ${PROJECT_SOURCE_DIR}/test)
configure_target(encrypter ${PROJECT_SOURCE_DIR}/test)
# configure_target(getPrime_test ${PROJECT_SOURCE_DIR}/test)

# make clean-all
add_custom_target(clean-all
    # COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/test
    # COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "清理所有构建文件和test文件夹"
)
