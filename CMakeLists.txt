cmake_minimum_required(VERSION 3.30)
project(rm_lines)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

include_directories(rm_lines/headers)

add_library(rm_lines SHARED rm_lines/src/library.cpp
        rm_lines/src/rm_lines.cpp
        rm_lines/src/reader/tagged_block_reader.cpp
        rm_lines/src/v5/reader.cpp
        rm_lines/src/v6/reader.cpp
        rm_lines/src/common/blocks.cpp
        rm_lines/src/common/scene_items.cpp
        rm_lines/src/common/data_types.cpp
        rm_lines/src/common/scene_tree.cpp
        rm_lines/src/common/crdt_sequence_item.cpp
        rm_lines/src/renderer/renderer.cpp
        rm_lines/src/renderer/renderer_export.cpp
)

target_link_libraries(rm_lines PRIVATE nlohmann_json::nlohmann_json)