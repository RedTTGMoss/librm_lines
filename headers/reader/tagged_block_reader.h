#ifndef TAGGED_BLOCK_READER_H
#define TAGGED_BLOCK_READER_H

#include <cstddef>
#include <cstdint>
#include <common/blocks.h>

class TaggedBlockReader {
public:
    TaggedBlockReader(void *data, const size_t data_size,
                      const int header_offset) : data_(static_cast<uint8_t *>(data)),
                                                 data_size_(data_size), current_offset(header_offset) {
    };

    ~TaggedBlockReader();

    virtual bool readHeader() = 0;

    virtual bool buildTree() = 0;

    bool readBlockInfo(BlockInfo &block_info);

    bool readBlock(Block *block, BlockInfo &block_info);

    uint8_t *data_;
    size_t data_size_;
    uint32_t current_offset;
};

#endif //TAGGED_BLOCK_READER_H
