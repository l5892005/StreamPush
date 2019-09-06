cmake_minimum_required(VERSION 3.4.1)
# 设置子目录相对于本CMakeLists.txt的位置
add_subdirectory(src/main/cpp/librtmp)
add_library(
        native-lib
        SHARED
        src/main/cpp/native-lib.cpp
        src/main/cpp/VideoChannel.cpp)
find_library(
        log-lib
        log)
# 引入x264头文件
include_directories(src/main/cpp/include)
# x264是用cpp编译的，引入libx264.a库
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/src/main/cpp/libs/${ANDROID_ABI}")
target_link_libraries(
        native-lib
        rtmp
        ${log-lib}
        x264)




