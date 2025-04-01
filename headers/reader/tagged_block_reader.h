#ifndef TAGGED_BLOCK_READER_H
#define TAGGED_BLOCK_READER_H

#include <cstddef>
#include <cstdint>
#include <common/blocks.h>

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

    bool hasBytesRemaining() const;
    size_t remainingBytes() const;

    // Read the blocks
    bool readBlockInfo();

    bool readBlock();

    // Read the sub blocks
    bool readSubBlock(uint8_t index, SubBlockInfo &subBlockInfo);

    bool readSubBlock(uint8_t index);

    bool checkSubBlock(uint8_t index);

    bool readValuint(uint64_t& result);

    bool readUUID(std::string& uuid, uint32_t length);

    bool readBytes(size_t size, void* result);

    bool checkTag(uint8_t expectedIndex, TagType expectedTagType);
    bool readTag(uint8_t expectedIndex, TagType expectedTagType);

    bool readId(uint8_t index, CrdtId *id);
    bool readId(CrdtId *id);
    bool readBool(uint8_t index, bool *result);
    bool readBool(bool *result);
    bool readInt(uint8_t index, uint32_t *result);
    bool readIntPair(uint8_t index, IntPair *result);
    bool readIntPair(IntPair *result);
    bool readFloat(uint8_t index, float *result);
    bool readFloat(float *result);
    bool readDouble(uint8_t index, double *result);
    bool readDouble(double *result);
    bool readByte(uint8_t index, uint8_t *result);
    bool readByte(uint8_t *result);
    bool readString(uint8_t index, std::string *result);
    bool readString(std::string *result);
    bool readColor(uint8_t index, Color *result);
    bool readColor(Color *result);

    // LWW values *Timestamped values
    bool readLwwId(uint8_t index, LwwItem<CrdtId> *id);
    bool readLwwBool(uint8_t index, LwwItem<bool> *result);
    bool readLwwFloat(uint8_t index, LwwItem<float> *result);
    bool readLwwByte(uint8_t index, LwwItem<uint8_t> *result);
    bool readLwwString(uint8_t index, LwwItem<std::string> *result);

    // Special
    bool readStringWithFormat(uint8_t index, StringWithFormat *result);
    bool readTextItem(TextItem *textItem);
    bool readTextFormat(TextFormat *textFormat);

    uint8_t *data_;
    size_t dataSize_;
    uint32_t currentOffset;
    Block* currentBlock;
    BlockInfo currentBlockInfo;
private:
    std::pair<uint8_t, TagType> _readTagValues();
    template <typename T>
    bool _readLwwTimestamp(uint8_t index, LwwItem<T> *id);
};

#endif //TAGGED_BLOCK_READER_H
