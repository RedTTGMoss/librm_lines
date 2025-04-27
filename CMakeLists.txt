cmake_minimum_required(VERSION 3.30)
project(rm_lines)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_Declare(
        cppcodec
        GIT_REPOSITORY https://github.com/tplgy/cppcodec.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(cppcodec)

include_directories(rm_lines/headers)

add_library(rm_lines SHARED rm_lines/src/library.cpp
        rm_lines/src/scene_tree/scene_tree_export.cpp
        rm_lines/src/reader/tagged_block_reader.cpp
        rm_lines/src/v5/reader.cpp
        rm_lines/src/v6/reader.cpp
        rm_lines/src/common/blocks.cpp
        rm_lines/src/common/scene_items.cpp
        rm_lines/src/common/data_types.cpp
        rm_lines/src/scene_tree/scene_tree.cpp
        rm_lines/src/common/crdt_sequence_item.cpp
        rm_lines/src/renderer/renderer.cpp
        rm_lines/src/renderer/renderer_export.cpp
        rm_lines/src/advanced/text_sequence.cpp
        rm_lines/src/advanced/text.cpp
)

target_link_libraries(rm_lines PRIVATE nlohmann_json::nlohmann_json cppcodec)

if (WIN32)
    target_link_libraries(rm_lines PRIVATE DbgHelp)
    set_target_properties(rm_lines PROPERTIES OUTPUT_NAME "rm_lines" PREFIX "")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
