#ifndef V5_READER_H
#define V5_READER_H

#include <library.h>

#include "reader/tagged_block_reader.h"
#define V5_HEADER "reMarkable .lines file, version=5          "
#define V5_HEADER_SIZE 43

class V5Reader : public TaggedBlockReader {
public:
    V5Reader(FILE* file, const size_t dataSize)
        : TaggedBlockReader(file, dataSize, V5_HEADER_SIZE) {
    };

    bool readHeader() override;
    bool readBlockInfo() override;
};

#endif //V5_READER_H
