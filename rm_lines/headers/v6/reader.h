#pragma once

#include <library.h>

#include "reader/tagged_block_reader.h"
#define V6_HEADER "reMarkable .lines file, version=6          "
#define V6_HEADER_SIZE 43

class V6Reader : public TaggedBlockReader {
public:
    V6Reader(FILE *file, const size_t dataSize)
        : TaggedBlockReader(file, dataSize, V6_HEADER_SIZE) {
    };

    bool readHeader() override;

    bool buildTree();
};
