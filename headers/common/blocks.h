#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class TaggedBlockReader;

struct BlockInfo {
    uint32_t offset;
    uint32_t size;
    uint8_t min_version;
    uint8_t current_version;
    uint8_t block_type;
};

struct SubBlockInfo {
    uint32_t offset;
    uint32_t size;
};

struct Block {
    Block();

    virtual ~Block();

    virtual bool read(TaggedBlockReader *reader, BlockInfo &info);

    static void lookup(Block *&block, const BlockInfo &info);
};

struct AuthorIdsBlock final : public Block {
    std::map<uint32_t, std::string> author_ids;
    bool read(TaggedBlockReader *reader, BlockInfo &info) override;
};

struct UnreadableBlock final : public Block {
};

#endif //BLOCKS_H
