
cmake_minimum_required(VERSION 3.27)
project(untitled)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(OpenCV_DIR E:\\opencv\\OpenCV-MinGW-Build-OpenCV-4.5.5-x64\\OpenCV-MinGW-Build-OpenCV-4.5.5-x64)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(E:\\MYSY32\\mingw64\\include)

add_executable(untitled
        main.cpp
        decode.cpp
        server.cpp
)

link_directories(${OpenCV_LIBRARY_DIRS})
link_directories(E:\\MYSY32\\mingw64\\lib)
target_link_libraries(untitled ${OpenCV_LIBS})
target_link_libraries(untitled ws2_32)

