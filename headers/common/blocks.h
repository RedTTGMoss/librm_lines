#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstdint>

class TaggedBlockReader;

struct BlockInfo {
    uint32_t offset;
    uint32_t size;
    uint8_t min_version;
    uint8_t current_version;
    uint8_t block_type;
};

struct Block {
    Block();

    ~Block();

    virtual bool read(TaggedBlockReader *reader, BlockInfo &info);

    static void lookup(const Block *block, const BlockInfo &info);
};

struct UnreadableBlock : public Block {
};

#endif //BLOCKS_H
