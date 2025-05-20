cmake_minimum_required(VERSION 3.30)
project(rm_lines)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_Declare(
        cppcodec
        GIT_REPOSITORY https://github.com/tplgy/cppcodec.git
        GIT_TAG master
)
FetchContent_Declare(
        utf8cpp
        GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
        GIT_TAG v4.0.6
)
FetchContent_MakeAvailable(utf8cpp)
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(cppcodec)

include_directories(rm_lines/headers)

file(GLOB RENDERER_FILES rm_lines/src/renderer/rm_lines_stroker/*.cpp)
file(GLOB RM_PENS_FILES rm_lines/src/renderer/rm_lines_stroker/rm_pens/*.cpp)
file(GLOB RM_TEMPLATE_FILES rm_lines/src/renderer/rm_lines_stroker/templates/*.cpp)
file(GLOB STB rm_lines/src/stb/*.cpp)

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
        ${STB}
        ${RENDERER_FILES}
        ${RM_PENS_FILES}
        ${RM_TEMPLATE_FILES}
)

target_link_libraries(rm_lines PRIVATE nlohmann_json::nlohmann_json cppcodec utf8cpp)

if (WIN32)
    target_link_libraries(rm_lines PRIVATE DbgHelp)
    set_target_properties(rm_lines PROPERTIES OUTPUT_NAME "rm_lines" PREFIX "")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(test tests/test.cpp)
target_link_libraries(test PRIVATE rm_lines)
target_link_libraries(test PRIVATE nlohmann_json::nlohmann_json cppcodec utf8cpp)

if (WIN32)
    target_link_libraries(test PRIVATE DbgHelp)
endif ()

target_compile_options(test PRIVATE -fsanitize=address)
target_link_options(test PRIVATE -fsanitize=address)
