cmake_minimum_required(VERSION 3.30)
project(rm_lines)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
set(SIMDUTF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SIMDUTF_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        simdutf
        GIT_REPOSITORY https://github.com/simdutf/simdutf.git
        GIT_TAG v8.2.0
)
FetchContent_MakeAvailable(simdutf)
FetchContent_MakeAvailable(utf8cpp)
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(cppcodec)

# Check python3 for the script later
find_package(Python3 COMPONENTS Interpreter REQUIRED)

include_directories(rm_lines/headers)

file(GLOB RENDERER_FILES rm_lines/src/renderer/rm_lines_stroker/*.cpp)
file(GLOB RM_PENS_FILES rm_lines/src/renderer/rm_lines_stroker/rm_pens/*.cpp)
file(GLOB RM_TEMPLATE_FILES rm_lines/src/renderer/rm_lines_stroker/templates/*.cpp)
file(GLOB STB rm_lines/src/stb/*.cpp)
set(RM_LINES_LIBRARY_TYPE SHARED)

# If building for web with emscripten, build a static library instead of a shared library
if (EMSCRIPTEN)
    set(RM_LINES_LIBRARY_TYPE STATIC)
endif ()

# Build the rm_lines library
add_library(rm_lines ${RM_LINES_LIBRARY_TYPE} rm_lines/src/library.cpp
        rm_lines/src/scene_tree/scene_tree_export.cpp
        rm_lines/src/reader/analyzer.cpp
        rm_lines/src/reader/tagged_block_reader.cpp
        rm_lines/src/writer/tagged_block_writer.cpp
        rm_lines/src/v5/reader.cpp
        rm_lines/src/v6/reader.cpp
        rm_lines/src/common/blocks.cpp
        rm_lines/src/common/scene_items.cpp
        rm_lines/src/common/data_types.cpp
        rm_lines/src/scene_tree/scene_tree.cpp
        rm_lines/src/scene_tree/scene_tree_editor.cpp
        rm_lines/src/common/crdt_sequence_item.cpp
        rm_lines/src/renderer/renderer.cpp
        rm_lines/src/renderer/image_renderer.cpp
        rm_lines/src/renderer/image_ref.cpp
        rm_lines/src/renderer/renderer_export.cpp
        rm_lines/src/advanced/text_sequence.cpp
        rm_lines/src/advanced/text.cpp
        ${STB}
        ${RENDERER_FILES}
        ${RM_PENS_FILES}
        ${RM_TEMPLATE_FILES}
)

target_link_libraries(rm_lines PRIVATE nlohmann_json::nlohmann_json cppcodec utf8cpp simdutf)

# Remove warnings about narrowing conversions in MSVC and GCC/Clang
if (MSVC)
    target_compile_options(rm_lines PRIVATE /wd4244 /wd4267)
else ()
    target_compile_options(rm_lines PRIVATE -Wno-narrowing)
endif ()

# Extract exported functions from rm_lines and generate exported_functions.txt
add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/exported_functions.txt
        COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/find_exports.py
        ${CMAKE_SOURCE_DIR}/rm_lines
        DEPENDS
        ${CMAKE_SOURCE_DIR}/tools/find_exports.py
        COMMENT "Extracting EXPORT functions"
)
add_custom_target(generate_exports DEPENDS ${CMAKE_BINARY_DIR}/exported_functions.txt)

if (WIN32)
    target_link_libraries(rm_lines PRIVATE DbgHelp)
    set_target_properties(rm_lines PROPERTIES OUTPUT_NAME "rm_lines" PREFIX "")
    #    target_compile_features(rm_lines PUBLIC cxx_std_20)
endif ()

# Building for web with emscripten
if (EMSCRIPTEN)
    add_executable(rm_lines_wasm rm_lines/src/wasm_entry.cpp)
    target_link_libraries(rm_lines_wasm PRIVATE rm_lines)
    add_dependencies(rm_lines_wasm generate_exports)
    target_link_libraries(rm_lines_wasm PRIVATE nlohmann_json::nlohmann_json cppcodec utf8cpp simdutf)

    target_link_options(rm_lines_wasm PRIVATE
            "SHELL:-s MODULARIZE=1"
            "SHELL:-s EXPORT_NAME=RMLines"
            "SHELL:-s ENVIRONMENT=web"
            "SHELL:-no-entry"
            "SHELL:$<IF:$<BOOL:1>,@${CMAKE_BINARY_DIR}/exported_functions.txt,>"

            "SHELL:-s ALLOW_MEMORY_GROWTH=1"
            "SHELL:-s ALLOW_TABLE_GROWTH=1"
            "SHELL:-O2"

            "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8','FS_writeFile','FS_readFile','FS_unlink','HEAPU8','HEAPU32','addFunction','removeFunction']"

            "SHELL:-gsource-map"
            "SHELL:-s ASSERTIONS=2"
    )
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(test tests/test.cpp)
target_link_libraries(test PRIVATE rm_lines)
target_link_libraries(test PRIVATE nlohmann_json::nlohmann_json cppcodec utf8cpp simdutf)

add_executable(test_writes tests/test_writes.cpp)
target_link_libraries(test_writes PRIVATE rm_lines)
target_link_libraries(test_writes PRIVATE nlohmann_json::nlohmann_json cppcodec utf8cpp simdutf)

if (WIN32)
    target_link_libraries(test PRIVATE DbgHelp)
    target_link_libraries(test_writes PRIVATE DbgHelp)
endif ()

target_compile_options(test PRIVATE -fsanitize=address)
target_link_options(test PRIVATE -fsanitize=address)

target_compile_options(test_writes PRIVATE -fsanitize=address)
target_link_options(test_writes PRIVATE -fsanitize=address)
