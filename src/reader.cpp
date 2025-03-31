#include "v6/reader.h"
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <format>
#include <library.h>

EXPORT size_t convertToSvg(int input_fd, size_t input_size, int output_fd) {
    if (input_size == 0) return 0;

    void *input_map = nullptr;
    void *output_map = nullptr;

    // Map input file
#ifdef _WIN32
    HANDLE hInputFile = (HANDLE)_get_osfhandle(input_fd);
    if (hInputFile == INVALID_HANDLE_VALUE) return 0;

    HANDLE hInputMapping = CreateFileMapping(hInputFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hInputMapping) return 0;

    input_map = MapViewOfFile(hInputMapping, FILE_MAP_READ, 0, 0, input_size);
    if (!input_map) {
        CloseHandle(hInputMapping);
        return 0;
    }
#else
    input_map = mmap(NULL, input_size, PROT_READ, MAP_SHARED, input_fd, 0);
    if (input_map == MAP_FAILED) return 0;
#endif

    // Initialize reader
    TaggedBlockReader *reader = new V6Reader(input_map, input_size);

    if (!reader->readHeader()) {
        // reader = V5Reader(input_map);
        if (!reader->readHeader()) {
#ifdef _WIN32
            UnmapViewOfFile(input_map);
            CloseHandle(hInputMapping);
#else
            munmap(input_map, input_size);
#endif
            return 0;
        }
    }

    const auto blockInfo = new BlockInfo();
    while (reader->readBlockInfo(*blockInfo)) {
        // logMessage(std::format("Read block info header {}:{} OF: {} S: {} BT: {}", blockInfo->min_version,
        //                        blockInfo->current_version, blockInfo->offset, blockInfo->size, blockInfo->block_type));
        uint32_t block_end = blockInfo->offset + blockInfo->size;
        if (Block *block = nullptr; !reader->readBlock(block, *blockInfo)) {
            reader->currentOffset = block_end;
            logMessage(std::format("Failed to read block type {}", blockInfo->block_type));
        } else if (reader->currentOffset < block_end) {
            logMessage(std::format("BLOCK DID NOT FULLY READ {} < {}", reader->currentOffset, block_end));
            reader->currentOffset = block_end;
        }
        else {
            logMessage(std::format("Read block {}", blockInfo->block_type));
        }
    }

    // Dummy SVG content (replace with real conversion later)
    const char *svg_content = "<svg xmlns=\"http://www.w3.org/2000/svg\"></svg>";
    size_t svg_size = strlen(svg_content);

    // Resize output file dynamically
#ifdef _WIN32
    HANDLE hOutputFile = (HANDLE)_get_osfhandle(output_fd);
    if (hOutputFile == INVALID_HANDLE_VALUE) {
        UnmapViewOfFile(input_map);
        CloseHandle(hInputMapping);
        return 0;
    }

    HANDLE hOutputMapping = CreateFileMapping(hOutputFile, NULL, PAGE_READWRITE, 0, svg_size, NULL);
    if (!hOutputMapping) {
        UnmapViewOfFile(input_map);
        CloseHandle(hInputMapping);
        return 0;
    }

    output_map = MapViewOfFile(hOutputMapping, FILE_MAP_WRITE, 0, 0, svg_size);
    if (!output_map) {
        CloseHandle(hOutputMapping);
        UnmapViewOfFile(input_map);
        CloseHandle(hInputMapping);
        return 0;
    }
#else
    if (ftruncate(output_fd, svg_size) != 0) {
        munmap(input_map, input_size);
        return 0;
    }

    output_map = mmap(NULL, svg_size, PROT_WRITE, MAP_SHARED, output_fd, 0);
    if (output_map == MAP_FAILED) {
        munmap(input_map, input_size);
        return 0;
    }
#endif

    // Write SVG data
    memcpy(output_map, svg_content, svg_size);

    // Cleanup
#ifdef _WIN32
    UnmapViewOfFile(input_map);
    CloseHandle(hInputMapping);
    UnmapViewOfFile(output_map);
    CloseHandle(hOutputMapping);
#else
    munmap(input_map, input_size);
    munmap(output_map, svg_size);
#endif

    return svg_size; // Return actual SVG size
}
