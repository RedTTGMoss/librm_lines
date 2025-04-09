#include "rm_lines.h"
#include "v5/reader.h"
#include "v6/reader.h"
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <format>
#include <library.h>
#include <memory>
#include <reader/tagged_block_reader.h>

EXPORT bool convertToSvg(const int inputFD, const int outputFD) {
    // Map input file
    size_t input_size = getFileSize(inputFD);

    // Initialize reader
    std::shared_ptr<TaggedBlockReader> reader = std::make_shared<V6Reader>(inputFD, input_size);

    if (!reader->readHeader()) {
        reader = std::make_shared<V5Reader>(inputFD, input_size);
        if (!reader->readHeader()) {
            return false;
        }
    }

    const auto tree = std::make_unique<SceneTree>();

    try {
        reader->buildTree(*tree.get());
    } catch (const std::exception &e) {
        logError(std::format("{}\nFailed to build tree: {}", getStackTrace(), e.what()));
    }

    // Dummy SVG content (replace with real conversion later)
    const char *svg_content = "<svg xmlns=\"http://www.w3.org/2000/svg\"></svg>";
    size_t svg_size = strlen(svg_content);

    // Write SVG data
    write(outputFD, svg_content, svg_size);

    return true;
}
