cmake_minimum_required(VERSION 3.15)
project(Webserver)


set(SERVER_SOURCES
    src/socket.cpp
    src/server.cpp
    src/threadpool.cpp
    src/httphandler.cpp
    src/logger.cpp
)

add_library(ServerEngine STATIC ${SERVER_SOURCES})
target_include_directories(ServerEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(webserver src/main.cpp)

target_link_libraries(webserver PRIVATE ServerEngine)