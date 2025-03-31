#ifndef TAGGED_BLOCK_READER_H
#define TAGGED_BLOCK_READER_H

#include <cstddef>
#include <cstdint>
#include <common/blocks.h>
#include <uuid/uuid.h>

enum class TagType : uint8_t {
    ID = 0xF,
    Length4 = 0xC,
    Byte8 = 0x8,
    Byte4 = 0x4,
    Byte2 = 0x2,
    Byte1 = 0x1,
    BAD = 0x0,
};

class TaggedBlockReader {
public:
    TaggedBlockReader(void *data, const size_t dataSize,
                      const int headerOffset) : data_(static_cast<uint8_t *>(data)),
                                                 dataSize_(dataSize), currentOffset(headerOffset) {
    };

    ~TaggedBlockReader();

    virtual bool readHeader() = 0;

    virtual bool buildTree() = 0;

    // Read the blocks
    bool readBlockInfo(BlockInfo &blockInfo);

    bool readBlock(Block *block, BlockInfo &blockInfo);

    bool readBlock(Block *block);

    // Read the sub blocks
    bool readSubBlock(uint8_t index, SubBlockInfo &subBlockInfo);

    bool readValuint(uint64_t& result);

    bool readUUID(std::string& uuid, const uint32_t length);

    bool readBytes(size_t size, void* result);

    bool readTag(const uint8_t expectedIndex, const TagType expectedTagType);

    uint8_t *data_;
    size_t dataSize_;
    uint32_t currentOffset;
private:
    std::pair<uint8_t, TagType> _readTagValues();
};

#endif //TAGGED_BLOCK_READER_H
