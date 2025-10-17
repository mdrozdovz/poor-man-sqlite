cmake_minimum_required(VERSION 3.28)
project(zero2hero C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(dbview-n
        include/common.h
        include/file.h
        include/parse.h
        src/file.c
        src/main.c
        src/parse.c)
