cmake_minimum_required(VERSION 3.30)
project(lib_rm_lines)

set(CMAKE_CXX_STANDARD 20)

include_directories(headers)

add_library(lib_rm_lines SHARED src/library.cpp
        src/reader.cpp
        src/reader/tagged_block_reader.cpp
        src/v6/reader.cpp
        )

set_target_properties(lib_rm_lines PROPERTIES OUTPUT_NAME "rm_lines")