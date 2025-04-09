#ifndef V6_READER_H
#define V6_READER_H

#include <library.h>

#include "reader/tagged_block_reader.h"
#define V6_HEADER "reMarkable .lines file, version=6          "
#define V6_HEADER_SIZE 43

class V6Reader : public TaggedBlockReader {
public:
    V6Reader(const int fd, const size_t data_size)
        : TaggedBlockReader(fd, data_size, V6_HEADER_SIZE) {
    };

    bool readHeader() override;
    bool buildTree();
};

#endif //V6_READER_H
