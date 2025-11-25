cmake_minimum_required(VERSION 3.28)
project(poor-man-sqlite C)

set(CMAKE_C_STANDARD 11)
set(PROJECT_SOURCE_DIR ../src)

include_directories(include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(
        dbview
        src/main.c
        src/file.c
        src/parse.c
        src/server.c
)

add_executable(
        client
        client/client.c
)
