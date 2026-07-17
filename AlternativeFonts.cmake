set(OPEN_SANS_FONT "${CMAKE_SOURCE_DIR}/rm_lines/assets/fonts/Karla-VariableFont_wght.ttf")
set(OPEN_SANS_ITALIC_FONT "${CMAKE_SOURCE_DIR}/rm_lines/assets/fonts/Karla-Italic-VariableFont_wght.ttf")
set(OPEN_SERIF_FONT "${CMAKE_SOURCE_DIR}/rm_lines/assets/fonts/LibreBaskerville-VariableFont_wght.ttf")
set(OPEN_SERIF_ITALIC_FONT "${CMAKE_SOURCE_DIR}/rm_lines/assets/fonts/LibreBaskerville-Italic-VariableFont_wght.ttf")

set(FONT_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/fonts")
set(FONT_HASH_DIR "${CMAKE_CURRENT_BINARY_DIR}/font_hashes")

file(MAKE_DIRECTORY "${FONT_GENERATED_DIR}")
file(MAKE_DIRECTORY "${FONT_HASH_DIR}")

set(FONT_SPLITTER "${CMAKE_SOURCE_DIR}/tools/split_variable_font.py")


function(split_font_if_needed NAME SOURCE OUTPUT_PREFIX)
    file(SHA256 "${SOURCE}" CURRENT_HASH)

    set(HASH_FILE "${FONT_HASH_DIR}/${NAME}.sha256")

    set(NEEDS_SPLIT FALSE)

    if (NOT EXISTS "${HASH_FILE}")
        set(NEEDS_SPLIT TRUE)
    else ()
        file(READ "${HASH_FILE}" OLD_HASH)

        if (NOT "${CURRENT_HASH}" STREQUAL "${OLD_HASH}")
            set(NEEDS_SPLIT TRUE)
        endif ()
    endif ()

    if (NOT EXISTS "${OUTPUT_PREFIX}_400.ttf")
        set(NEEDS_SPLIT TRUE)
    endif ()

    if (NEEDS_SPLIT)
        message(STATUS "Generating font weights for ${NAME}")

        execute_process(
                COMMAND
                ${Python3_EXECUTABLE}
                "${FONT_SPLITTER}"
                "${SOURCE}"
                "${OUTPUT_PREFIX}"
                RESULT_VARIABLE RESULT
        )

        if (NOT RESULT EQUAL 0)
            message(FATAL_ERROR "Failed generating ${NAME}")
        endif ()

        file(WRITE "${HASH_FILE}" "${CURRENT_HASH}")
    endif ()
endfunction()

split_font_if_needed(
        sans
        "${OPEN_SANS_FONT}"
        "${FONT_GENERATED_DIR}/sans"
)

split_font_if_needed(
        sans_italic
        "${OPEN_SANS_ITALIC_FONT}"
        "${FONT_GENERATED_DIR}/sans_italic"
)

split_font_if_needed(
        serif
        "${OPEN_SERIF_FONT}"
        "${FONT_GENERATED_DIR}/serif"
)

split_font_if_needed(
        serif_italic
        "${OPEN_SERIF_ITALIC_FONT}"
        "${FONT_GENERATED_DIR}/serif_italic"
)

embed_resource(
        "${FONT_GENERATED_DIR}/sans_300.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_300.h"
        sans_300FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_400.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_400.h"
        sans_400FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_500.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_500.h"
        sans_500FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_700.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_700.h"
        sans_700FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_800.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_800.h"
        sans_800FontData
)

embed_resource(
        "${FONT_GENERATED_DIR}/sans_italic_300.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_italic_300.h"
        sans_italic_300FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_italic_400.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_italic_400.h"
        sans_italic_400FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_italic_500.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_italic_500.h"
        sans_italic_500FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_italic_700.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_italic_700.h"
        sans_italic_700FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/sans_italic_800.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/sans_italic_800.h"
        sans_italic_800FontData
)

embed_resource(
        "${FONT_GENERATED_DIR}/serif_400.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/serif_400.h"
        serif_400FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/serif_700.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/serif_700.h"
        serif_700FontData
)

embed_resource(
        "${FONT_GENERATED_DIR}/serif_italic_400.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/serif_italic_400.h"
        serif_italic_400FontData
)
embed_resource(
        "${FONT_GENERATED_DIR}/serif_italic_700.ttf"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/serif_italic_700.h"
        serif_italic_700FontData
)

message(STATUS "Using open source fonts (EB Garamond and Noto Sans) instead of reMarkable fonts.")