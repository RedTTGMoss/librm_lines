#pragma once
#include <cstdint>
#include <cstdio>

#include "renderer/renderer.h"

constexpr uint8_t BYTE_ONE = 1;
constexpr uint8_t BYTE_ZERO = 0;
constexpr uint32_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();

enum class TagType : uint8_t;

class TaggedBlockWriter {
public:
    std::ostream &stream;
    size_t dataSize_;
    uint32_t currentOffset = 0;
    const Renderer *renderer;

    TaggedBlockWriter(std::ostream &stream, const Renderer *renderer) : stream(stream), renderer(renderer) {
    };

    ~TaggedBlockWriter();

    bool buildRM();

    bool seekTo(uint64_t offset);

    bool writeBytes(uint32_t size, const void *data);

    bool writeBlock(const Block &block) {
        if (!block.info.has_value()) {
            logError("Block missing info");
            return false;
        }
        auto startOffset = writeBlockInfoHeaderStart(block.info.value());
        if (startOffset < 0) return false;
        if (!block.write(this)) {
            logError(std::format("Failed to write block of type {}", block.info->blockType));
            return false;
        }
        writeBlockInfoHeaderEnd(startOffset);
        logDebug(std::format("Successfully wrote block of type {}", block.info->blockType));
        return true;
    }

    template<typename T>
    void writeObj(T value) {
        writeBytes(sizeof(T), &value);
    }

    // Begin types
    bool writeLwwId(uint8_t index, const LwwItem<CrdtId> *id);

    bool writeLwwBool(uint8_t index, const LwwItem<bool> *value);

    bool writeLwwRectPair(uint8_t index, const LwwItem<RectPair> *value);

    bool writeLwwDoublePair(uint8_t index, const LwwItem<DoublePair> *value);

    bool writeLwwByte(uint8_t index, const LwwItem<uint8_t> *value);

    template<typename T>
    uint32_t _writeLwwItemId(uint8_t index, const LwwItem<T> *id);

    bool writeTag(uint8_t index, TagType type);

    [[nodiscard]] uint32_t writeSubBlockStart(uint8_t index);

    bool writeSubBlockEnd(uint32_t subBlockStart);

    bool writeValuint(uint64_t value);

    bool writeId(uint8_t index, const CrdtId *id);

    bool writeId(const CrdtId *id);

    bool writeBool(uint8_t index, const bool &value);

    bool writeBool(const bool &value);

    bool writeByte(uint8_t index, const uint8_t *value);

    bool writeByte(const uint8_t *value);

    bool writeInt(uint8_t index, const uint32_t *value);

    bool writeInt(const uint32_t *value);

    bool writeIntPair(uint8_t index, const IntPair *value);

    bool writeIntPair(const IntPair *value);

    bool writeRectPair(uint8_t index, const RectPair *value);

    bool writeRectPair(const RectPair *value);

    bool writeDoublePair(uint8_t index, const DoublePair *value);

    bool writeDoublePair(const DoublePair *value);

    bool writeUUID(const std::string &uuid);

    [[nodiscard]] uint32_t writeBlockInfoHeaderStart(const BlockInfo &blockInfo);

    bool writeBlockInfoHeaderEnd(uint32_t blockStartOffset);
};
