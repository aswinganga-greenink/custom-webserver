cmake_minimum_required(VERSION 3.15)
project(Webserver)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(ENABLE_ASAN "Enable Address Sanitizer for memory auditing" OFF)

if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer is ENABLED. Performance will be degraded.")

    add_compile_options(-fsanitize=address -g -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

set(SERVER_SOURCES
    src/socket.cpp
    src/server.cpp
    src/threadpool.cpp
    src/httphandler.cpp
    src/httprequest.cpp
    src/logger.cpp
    src/epoll.cpp
    src/config.cpp
)

add_library(ServerEngine STATIC ${SERVER_SOURCES})
target_include_directories(ServerEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(webserver src/main.cpp)

target_link_libraries(webserver PRIVATE ServerEngine)


include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(server_tests tests/test_config.cpp tests/test_http_parser.cpp)

target_sources(server_tests PRIVATE src/logger.cpp)
target_link_libraries(server_tests gtest_main ServerEngine)

include(GoogleTest)
gtest_discover_tests(server_tests)