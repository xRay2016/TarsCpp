# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_LIB_SRCS 变量
#aux_source_directory(. DIR_LIB_SRCS)

file(GLOB DIR_SRCS *.cpp)
file(GLOB DIR_HEADERS *.h)

message(STATUS ${DIR_SRCS})
message(STATUS ${DIR_HEADERS})


# 生成链接库
add_library (LIB_SRCS ${DIR_SRCS} ${DIR_HEADERS})