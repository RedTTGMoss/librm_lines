#pragma once
#include <cstdint>
#include <cstdio>
#include <simdutf.h>
#include <string_view>
#include "renderer/renderer.h"

namespace writer {
    constexpr uint8_t BYTE_ONE = 1;
    constexpr uint8_t BYTE_ZERO = 0;
    constexpr uint8_t BYTE_SEVENTEEN = 17;
    constexpr uint32_t MAX_UINT32 = std::numeric_limits<uint32_t>::max();
    static const std::string EMPTY_STRING{};
}

enum class TagType : uint8_t;

inline bool is_utf8(const std::string_view s) {
    return simdutf::validate_utf8(s);
}

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

    bool writeBlock(const Block *block);

    bool writeNode(const Group *node);

    bool writeRootText(const Text &text);

    template<typename T>
    void writeObj(T value) {
        writeBytes(sizeof(T), &value);
    }

    // Begin types
    bool writeLwwId(uint8_t index, const LwwItem<CrdtId> *id);

    bool writeLwwBool(uint8_t index, const LwwItem<bool> *value);

    bool writeLwwFloat(uint8_t index, const LwwItem<float> *value);

    bool writeLwwRectPair(uint8_t index, const LwwItem<RectPair> *value);

    bool writeLwwDoublePair(uint8_t index, const LwwItem<DoublePair> *value);

    bool writeLwwByte(uint8_t index, const LwwItem<uint8_t> *value);

    bool writeLwwByteSub(uint8_t index, const LwwItem<uint8_t> *value);

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

    bool writeByteSub(uint8_t index, const uint8_t *value);

    bool writeByte(const uint8_t *value);


    bool writeInt(uint8_t index, const uint32_t *value);

    bool writeInt(const uint32_t *value);

    bool writeDouble(uint8_t index, const double *value);

    bool writeDouble(const double *value);

    bool writeFloat(uint8_t index, const float *value);

    bool writeFloat(const float *value);

    bool writeString(uint8_t index, const std::string *value);

    bool writeString(const std::string *value);

    bool writeEmptyString(const uint8_t index) {
        return writeString(index, &writer::EMPTY_STRING);
    }

    bool writeEmptyString() {
        return writeString(&writer::EMPTY_STRING);
    }

    bool writeIntPair(uint8_t index, const IntPair *value);

    bool writeIntPair(const IntPair *value);

    bool writeRectPair(uint8_t index, const RectPair *value);

    bool writeRectPair(const RectPair *value);

    bool writeDoublePair(uint8_t index, const DoublePair *value);

    bool writeDoublePair(const DoublePair *value);

    bool writeUUID(const std::string &uuid);

    bool writeTextItem(const TextItem *textItem);

    bool writeTextFormat(const TextFormat *textFormat);

    [[nodiscard]] uint32_t writeBlockInfoHeaderStart(const BlockInfo &blockInfo);

    bool writeBlockInfoHeaderEnd(uint32_t blockStartOffset);
};
