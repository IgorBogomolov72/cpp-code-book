cmake_minimum_required(VERSION 3.10)
project(SimpleServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3)
add_executable(server src/server.cpp src/database.cpp)
target_link_libraries(server PRIVATE SQLite::SQLite3)