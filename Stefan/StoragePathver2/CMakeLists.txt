cmake_minimum_required(VERSION 3.20)
project(StoragePathver2)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -g -fsanitize=undefined,bounds,address -D_GLIBCXX_DEBUG")
find_package(nlohmann_json 3.10.4 REQUIRED)
add_executable(StoragePathver2 main.cpp)
target_link_libraries(StoragePathver2 PRIVATE nlohmann_json::nlohmann_json)

