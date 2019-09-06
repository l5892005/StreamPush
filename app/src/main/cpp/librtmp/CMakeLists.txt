cmake_minimum_required(VERSION 3.4.1)
# 告诉c++编译器  此文件由c来编译，加-D后面接参数NO_CRYPTO。传入宏定义
# 加入-DNO_CRYPTO，关闭ssl 不支持rtmps
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")
# 下面这句等同于 file(GLOB rtmp_source *.c)
#file(GLOB rtmp_source *.c)
#aux_source_directory(. rtmp_source)
add_library(
        rtmp
        STATIC
        amf.c
        hashswf.c
        log.c
        parseurl.c
        rtmp.c)