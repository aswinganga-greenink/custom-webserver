cmake_minimum_required(VERSION 4.3.1)
project(Webserver)

add_library(Socket STATIC src/socket.cpp)
target_include_directories(Socket PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Server STATIC src/server.cpp)
target_include_directories(Server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(ThreadPool STATIC src/threadpool.cpp)
target_include_directories(ThreadPool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Server PUBLIC Socket ThreadPool)

add_executable(webserver src/main.cpp)
target_link_libraries(webserver PRIVATE Server)