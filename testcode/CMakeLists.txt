# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.0.0)  
# 项目名称
project(main)


if("${TARS_CPP_COMMON}" STREQUAL "")

    set(PROJECT_DIR /root/tars/TarsFramework/tarscpp)

    set(TARS_CPP_COMMON "1")

    include("${PROJECT_DIR}/cmake/GenVersion.cmake")

    include("${PROJECT_DIR}/cmake/Common.cmake")

    include("${PROJECT_DIR}/cmake/BuildTarsCpp.cmake")

    include("${PROJECT_DIR}/cmake/Thirdparty.cmake")

    include("${PROJECT_DIR}/cmake/ParseAsm.cmake")

endif()


macro(parse_asm JUMP_SRC MAKE_SRC)
set(JUMP_SRC "asm/jump_x86_64_sysv_elf_gas.S")
set(MAKE_SRC "asm/make_x86_64_sysv_elf_gas.S")
endmacro ()


#向工程添加多个特定的头文件搜索路径,路径之间用空格分隔
include_directories(include)
include_directories(src)
#include_directories(/root/tars/TarsFramework/tarscpp/util/include/util)

set(TARSCPP_PATH /root/tars/TarsFramework/tarscpp/)
include_directories(${TARSCPP_PATH}util/include)
include_directories(${TARSCPP_PATH}servant)
include_directories(${TARSCPP_PATH}servant/tup)

#指定生成可执行文件的目录   PROJECT_BINARY_DIR 运行cmake命令的目录,通常是${PROJECT_SOURCE_DIR}/build   
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

SET(CMAKE_CXX_STANDARD 11)

#添加 src 子目录 
add_subdirectory(src)
add_subdirectory(${TARSCPP_PATH} tarsutil)

set(ENDPOINT_FILES include/EndpointF.h include/EndpointInfo.h)
# 指定生成目标 将名为 main.cpp 的源文件编译成一个名称为 ${PROJECT_NAME} 的可执行文件。
add_executable(${PROJECT_NAME} main.cpp ${ENDPOINT_FILES})   


# 添加链接库  可执行文件 ${PROJECT_NAME} 需要连接一个名为 Lib 的链接库 
target_link_libraries(${PROJECT_NAME} LIB_SRCS)
target_link_libraries(${PROJECT_NAME} tarsutil)