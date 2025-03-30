#ifndef v6_READER_H
#define v6_READER_H

#include <library.h>

#include "reader/tagged_block_reader.h"
#define V6_HEADER "reMarkable .lines file, version=6          "
#define V6_HEADER_SIZE 43

class V6Reader : public TaggedBlockReader {
public:
    V6Reader(void *data, const size_t data_size)
        : TaggedBlockReader(data, data_size) {
    };

    bool readHeader() override;
    bool compileTree() override;
protected:
    int current_offset = V6_HEADER_SIZE;
};

#endif //v6_READER_H
