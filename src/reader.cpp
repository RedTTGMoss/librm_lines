#include "v6/reader.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <format>
#include <library.h>

EXPORT size_t convertToSvg(int input_fd, size_t input_size, int output_fd) {
    if (input_size == 0) return 0;

    // Map input file
    void *input_map = mmap(NULL, input_size, PROT_READ, MAP_SHARED, input_fd, 0);
    if (input_map == MAP_FAILED) return 0;

    // Initialize reader
    TaggedBlockReader *reader = new V6Reader(input_map, input_size);

    if (!reader->readHeader()) {
        // reader = V5Reader(input_map);
        if (!reader->readHeader()) {
            munmap(input_map, input_size);
            return 0;
        }
    }

    auto *block = new BlockInfo();
    while (reader->readBlock(*block)) {
        logMessage(std::format(
            "Block min_version: {}, current_version: {}, block_type: {}",
            block->min_version, block->current_version, block->block_type));
        reader->skipBytes(block->size);
    }


    // Dummy SVG content (replace with real conversion later)
    const char *svg_content = "<svg xmlns=\"http://www.w3.org/2000/svg\"></svg>";
    size_t svg_size = strlen(svg_content);

    // Resize output file dynamically
    if (ftruncate(output_fd, svg_size) != 0) {
        munmap(input_map, input_size);
        return 0;
    }

    // Map output file
    void *output_map = mmap(NULL, svg_size, PROT_WRITE, MAP_SHARED, output_fd, 0);
    if (output_map == MAP_FAILED) {
        munmap(input_map, input_size);
        return 0;
    }

    // Write SVG data
    memcpy(output_map, svg_content, svg_size);

    // Cleanup
    munmap(input_map, input_size);
    munmap(output_map, svg_size);

    return svg_size; // Return actual SVG size
}
