cmake_minimum_required(VERSION 3.20)
project(StoragePathver2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -g -fsanitize=undefined,bounds,address -D_GLIBCXX_DEBUG")


add_executable(StoragePathver2 main.cpp)
