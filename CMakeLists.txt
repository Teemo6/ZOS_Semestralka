cmake_minimum_required(VERSION 3.16)

project(ZOS_Semestralka)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

add_executable(
        ZOS_Semestralka
        Constants.hpp
        Main.cpp
        FileSystem.cpp
        FileSystem.hpp
        Superblock.cpp
        Superblock.hpp
        DirectoryItem.cpp
        DirectoryItem.hpp
        IndexNode.cpp
        IndexNode.hpp
        Bitmap.cpp Bitmap.hpp)
