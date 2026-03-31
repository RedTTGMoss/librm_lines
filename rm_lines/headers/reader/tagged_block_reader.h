#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <common/blocks.h>
#include <scene_tree/scene_tree.h>
#include <common/crdt_sequence_item.h>

enum class TagType : uint8_t {
    ID = 0xF,
    Length4 = 0xC,
    Byte8 = 0x8,
    Byte4 = 0x4,
    Byte2 = 0x2,
    Byte1 = 0x1,
    BAD_LENGTH = 0x0,
    BAD_VALUINT = 0xFF,
};

inline std::string debugTagTypes(TagType type) {
    switch (type) {
        case TagType::ID:
            return "ID";
        case TagType::Length4:
            return "Length4";
        case TagType::Byte8:
            return "Byte8";
        case TagType::Byte4:
            return "Byte4";
        case TagType::Byte2:
            return "Byte2";
        case TagType::Byte1:
            return "Byte1";
        case TagType::BAD_LENGTH:
            return "BAD_LENGTH";
        case TagType::BAD_VALUINT:
            return "BAD_VALUINT";
        default:
            return std::format("Unknown(0x{:X})", static_cast<unsigned int>(type));
    }
}

struct Tag {
    uint8_t index;
    TagType type;
};

class TaggedBlockReader {
public:
    FILE *file;
    size_t dataSize_;
    uint32_t currentOffset;
    std::unique_ptr<Block> currentBlock;
    BlockInfo currentBlockInfo;

    TaggedBlockReader(FILE *file, const size_t dataSize,
                      const int headerOffset) : file(file),
                                                dataSize_(dataSize), currentOffset(headerOffset) {
    };

    ~TaggedBlockReader();

    virtual bool readHeader() = 0;

    bool buildTree(SceneTree &tree);

    bool hasBytesRemaining() const;

    uint32_t remainingBytes() const;

    // Read the blocks
    virtual bool readBlockInfo();

    bool readBlock();

    // Read the sub blocks
    bool readSubBlock(uint8_t index, SubBlockInfo &subBlockInfo);

    bool readSubBlock(uint8_t index);

    bool checkSubBlock(uint8_t index) const;

    bool debugSubBlock(uint8_t index);

    bool readValuint(uint64_t &result);

    bool readUUID(std::string &uuid, uint32_t length);

    bool readBytes(uint32_t size, void *result);

    void readBytesOrError(uint32_t size, void *result);

    void skipBytes(uint32_t size);

    void seekTo(uint32_t offset);

    bool checkTag(uint8_t expectedIndex, TagType expectedTagType) const;

    bool checkRequiredTag(uint8_t expectedIndex, TagType expectedTagType, bool error = true);

    void getTag();

    bool debugTag(uint8_t padding = 0);

    void claimTag();

    bool readId(uint8_t index, CrdtId *id);

    bool readId(CrdtId *id);

    bool readBool(uint8_t index, bool *result);

    bool readBool(bool *result);

    bool readInt(uint8_t index, uint32_t *result);

    bool readInt(uint32_t *result);

    bool readIntPair(uint8_t index, IntPair *result);

    bool readIntPair(IntPair *result);

    bool readDoublePair(uint8_t index, DoublePair *result);

    bool readDoublePair(DoublePair *result);

    bool readFloat(uint8_t index, float *result);

    bool readFloat(float *result);

    bool readDouble(uint8_t index, double *result);

    bool readDouble(double *result);

    bool readByte(uint8_t index, uint8_t *result);

    bool readBytesIndexed(uint8_t index, uint32_t size, std::vector<uint8_t> *result);

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

    bool readLwwDoublePair(uint8_t index, LwwItem<DoublePair> *result);

    bool readLwwBytes(uint8_t index, LwwItem<std::vector<uint8_t> > *result);

    bool readLwwString(uint8_t index, LwwItem<std::string> *result);

    // Special
    bool readStringWithFormat(uint8_t index, StringWithFormat *result);

    bool readTextItem(TextItem *textItem);

    bool readTextFormat(TextFormat *textFormat);

private:
    std::pair<uint8_t, TagType> _readTagValues();

    template<typename T>
    bool _readLwwItemId(uint8_t index, LwwItem<T> *id);

    template<typename T>
    std::optional<SubBlockInfo> _readLwwItemIdInfo(uint8_t index, LwwItem<T> *id);

    Tag tag;
    bool tagClaimed = true;
};

class CannotReadHeaderException : public std::exception {
public:
    explicit CannotReadHeaderException(const std::string &message)
        : message_("CannotReadHeaderException: " + message) {
    }

    const char *what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};
