#include "writer/tagged_block_writer.h"
#include "v6/reader.h"

TaggedBlockWriter::~TaggedBlockWriter() {
}

bool TaggedBlockWriter::buildRM() {
    writeBytes(V6_HEADER_SIZE, V6_HEADER);
    // Author IDs block
    if (renderer->sceneTree->authorsInfo.has_value()) {
        if (!writeBlock(&renderer->sceneTree->authorsInfo.value())) return false;
    }
    // Migration info block
    if (renderer->sceneTree->migrationInfo.has_value()) {
        if (!writeBlock(&renderer->sceneTree->migrationInfo.value())) return false;
    }
    // Page info block
    if (renderer->sceneTree->pageInfo.has_value()) {
        if (!writeBlock(&renderer->sceneTree->pageInfo.value())) return false;
    }
    // Scene info block
    if (renderer->sceneTree->sceneInfo.has_value()) {
        if (!writeBlock(&renderer->sceneTree->sceneInfo.value())) return false;
    }
    // Image info block
    if (renderer->sceneTree->imageInfo.has_value()) {
        if (!writeBlock(&renderer->sceneTree->imageInfo.value())) return false;
    }
    // Start writing nodes
    for (auto &node: renderer->sceneTree->_nodeIds | std::views::values) {
        if (!writeNode(node.get())) return false;
    }

    if (!writeRootText(renderer->textDocument.toText())) return false;

    return true;
}

bool TaggedBlockWriter::seekTo(const uint64_t offset) {
    stream.clear(); // clear eof/fail bits if needed

    stream.seekp(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!stream) {
        logError(std::format("Failed to seek to offset {}", offset));
        return false;
    }

    currentOffset = stream.tellp();
    return true;
}

bool TaggedBlockWriter::writeBytes(const uint32_t size, const void *data) {
    if (size == 0)
        return true; // nothing to do, skip.

    // Try writing to stream
    if (!stream.write(static_cast<const char *>(data), size)) {
        logError(std::format("Failed to write {} bytes at offset {}", size, currentOffset));
        return false;
    }

    // Update the current offset
    currentOffset = stream.tellp();
    return true;
}

bool TaggedBlockWriter::writeBlock(const Block *block) {
    if (!block->info.has_value()) {
        logError("Block missing info");
        return false;
    }
    uint32_t startOffset;
    if (startOffset = writeBlockInfoHeaderStart(block->info.value()); startOffset == 0) return false;
    if (!block->write(this)) {
        logError(std::format("Failed to write block of type {}", block->info->blockType));
        return false;
    }
    writeBlockInfoHeaderEnd(startOffset);
    logDebug(std::format("Successfully wrote block of type {}", block->info->blockType));
    return true;
}

bool TaggedBlockWriter::writeNode(const Group *node) {
    if (node->nodeId == ROOT_NODE) return true; // Root node is derived.

    const SceneTreeBlock block = SceneTreeBlock::fromNode(node);
    return writeBlock(&block);
}

bool TaggedBlockWriter::writeRootText(const Text &text) {
    const RootTextBlock block = RootTextBlock::fromText(text);
    return writeBlock(&block);
}

bool TaggedBlockWriter::writeLwwId(const uint8_t index, const LwwItem<CrdtId> *id) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<CrdtId>(index, id); subBlockStart == 0) return false;
    if (!writeId(2, &id->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwBool(const uint8_t index, const LwwItem<bool> *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<bool>(index, value); subBlockStart == 0) return false;
    if (!writeBool(2, value->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwFloat(const uint8_t index, const LwwItem<float> *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<float>(index, value); subBlockStart == 0) return false;
    if (!writeFloat(2, &value->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwRectPair(const uint8_t index, const LwwItem<RectPair> *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<RectPair>(index, value); subBlockStart == 0) return false;
    if (!writeRectPair(2, &value->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwDoublePair(const uint8_t index, const LwwItem<DoublePair> *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<DoublePair>(index, value); subBlockStart == 0) return false;
    if (!writeDoublePair(2, &value->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwByte(const uint8_t index, const LwwItem<uint8_t> *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<uint8_t>(index, value); subBlockStart == 0) return false;
    if (!writeByte(2, &value->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwBytes(const uint8_t index, const LwwItem<std::vector<uint8_t> > *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<std::vector<uint8_t> >(index, value); subBlockStart == 0) return false;
    uint32_t valueBlockStart;
    if (valueBlockStart = writeSubBlockStart(2); valueBlockStart == 0) return false;
    if (!writeBytes(value->value.size(), value->value.data())) return false;
    if (!writeSubBlockEnd(valueBlockStart)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwString(const uint8_t index, const LwwItem<std::string> *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<std::string>(index, value); subBlockStart == 0) return false;
    if (!writeString(2, &value->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeLwwByteSub(const uint8_t index, const LwwItem<uint8_t> *value) {
    uint32_t subBlockStart;
    if (subBlockStart = _writeLwwItemId<uint8_t>(index, value); subBlockStart == 0) return false;
    if (!writeByteSub(2, &value->value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeTag(const uint8_t index, TagType type) {
    const uint64_t x =
            (static_cast<uint64_t>(index) << 4) |
            (static_cast<uint64_t>(type) & 0xF);

    return writeValuint(x);
}

uint32_t TaggedBlockWriter::writeSubBlockStart(const uint8_t index) {
    if (!writeTag(index, TagType::Length4)) return -1;
    const uint32_t startOffset = currentOffset;
    if (!writeBytes(sizeof(uint32_t), &writer::MAX_UINT32)) return 0;

    return startOffset;
}

bool TaggedBlockWriter::writeSubBlockEnd(const uint32_t subBlockStart) {
    const uint32_t size = currentOffset - subBlockStart - 4;
    const uint32_t returnOffset = currentOffset;
    seekTo(subBlockStart);
    if (!writeBytes(sizeof(uint32_t), &size)) return false;
    seekTo(returnOffset);
    return true;
}

bool TaggedBlockWriter::writeValuint(uint64_t value) {
    while (true) {
        uint8_t byte = static_cast<uint8_t>(value & 0x7F);
        value >>= 7;

        if (value != 0) {
            byte |= 0x80;
        }

        if (!writeBytes(1, &byte)) {
            return false;
        }

        if ((byte & 0x80) == 0) {
            return true;
        }
    }
}

bool TaggedBlockWriter::writeId(const uint8_t index, const CrdtId *id) {
    if (!writeTag(index, TagType::ID)) return false;
    return writeId(id);
}

bool TaggedBlockWriter::writeId(const CrdtId *id) {
    if (!writeBytes(sizeof(id->first), &id->first)) return false;
    return writeValuint(id->second);
}

bool TaggedBlockWriter::writeBool(const uint8_t index, const bool &value) {
    if (!writeTag(index, TagType::Byte1)) return false;
    return writeBool(value);
}

bool TaggedBlockWriter::writeBool(const bool &value) {
    return writeBytes(sizeof(uint8_t), value ? &writer::BYTE_ONE : &writer::BYTE_ZERO);
}

bool TaggedBlockWriter::writeByte(const uint8_t index, const uint8_t *value) {
    if (!writeTag(index, TagType::Byte1)) return false;
    return writeByte(value);
}

bool TaggedBlockWriter::writeByteSub(const uint8_t index, const uint8_t *value) {
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(index); subBlockStart == 0) return false;
    if (!writeByte(value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeByte(const uint8_t *value) {
    return writeBytes(sizeof(uint8_t), value);
}

bool TaggedBlockWriter::writeInt(const uint8_t index, const uint32_t *value) {
    if (!writeTag(index, TagType::Byte4)) return false;
    return writeInt(value);
}

bool TaggedBlockWriter::writeInt(const uint32_t *value) {
    return writeBytes(sizeof(uint32_t), value);
}

bool TaggedBlockWriter::writeDouble(const uint8_t index, const double *value) {
    if (!writeTag(index, TagType::Byte8)) return false;
    return writeDouble(value);
}

bool TaggedBlockWriter::writeDouble(const double *value) {
    return writeBytes(sizeof(double), value);
}

bool TaggedBlockWriter::writeFloat(const uint8_t index, const float *value) {
    if (!writeTag(index, TagType::Byte4)) return false;
    return writeFloat(value);
}

bool TaggedBlockWriter::writeFloat(const float *value) {
    return writeBytes(sizeof(float), value);
}

bool TaggedBlockWriter::writeString(const uint8_t index, const std::string *value) {
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(index); subBlockStart == 0) return false;
    if (!writeString(value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeString(const std::string *value) {
    const uint64_t stringLength = value->length();
    constexpr bool isAscii = true;
    if (!writeValuint(stringLength)) return false;
    if (!writeBool(isAscii)) return false;
    return writeBytes(static_cast<uint32_t>(stringLength), value->data());
}

bool TaggedBlockWriter::writeIntPair(const uint8_t index, const IntPair *value) {
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(index); subBlockStart == 0) return false;
    if (!writeIntPair(value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeIntPair(const IntPair *value) {
    return writeBytes(sizeof(IntPair), value);
}

bool TaggedBlockWriter::writeRectPair(const uint8_t index, const RectPair *value) {
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(index); subBlockStart == 0) return false;
    if (!writeRectPair(value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeRectPair(const RectPair *value) {
    return writeBytes(sizeof(RectPair), value);
}

bool TaggedBlockWriter::writeDoublePair(const uint8_t index, const DoublePair *value) {
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(index); subBlockStart == 0) return false;
    if (!writeDoublePair(value)) return false;
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeDoublePair(const DoublePair *value) {
    return writeBytes(sizeof(DoublePair), value);
}

bool TaggedBlockWriter::writeUUID(const std::string &uuid) {
    std::string hex;
    hex.reserve(32);

    for (char c: uuid) {
        if (c == '-')
            continue;

        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            logError(std::format("Invalid UUID character: '{}'", c));
            return false;
        }

        hex.push_back(c);
    }

    if (hex.size() != 32) {
        logError(std::format("UUID must contain exactly 32 hex digits, got {}", hex.size()));
        return false;
    }

    auto hexValue = [](const char c) -> uint8_t {
        if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(10 + c - 'a');
        if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(10 + c - 'A');
        return 0;
    };

    std::array<uint8_t, 16> bytes{};

    for (size_t i = 0; i < 16; ++i) {
        bytes[i] = static_cast<uint8_t>(
            (hexValue(hex[i * 2]) << 4) |
            hexValue(hex[i * 2 + 1])
        );
    }

    return writeBytes(16, bytes.data());
}

bool TaggedBlockWriter::writeTextItem(const TextItem *textItem) {
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(0); subBlockStart == 0) return false;

    // Write items
    if (!writeId(2, &textItem->itemId)) return false;
    if (!writeId(3, &textItem->leftId)) return false;
    if (!writeId(4, &textItem->rightId)) return false;
    if (!writeInt(5, &textItem->deletedLength)) return false;

    // The value either contains a string or a format
    auto value = textItem->value.value_or("");

    // Check variant
    if (std::holds_alternative<std::string>(value)) {
        const auto string = std::get<std::string>(value);
        if (!string.empty()) {
            uint32_t valueSubBlockStart;
            if (valueSubBlockStart = writeSubBlockStart(6); valueSubBlockStart == 0) return false;
            // Write the text
            if (!writeString(&string)) return false;
            if (!writeSubBlockEnd(valueSubBlockStart)) return false;
        }
    } else if (std::holds_alternative<uint32_t>(value)) {
        const uint32_t format = std::get<uint32_t>(value);
        uint32_t valueSubBlockStart;
        if (valueSubBlockStart = writeSubBlockStart(6); valueSubBlockStart == 0) return false;
        // Write empty text
        if (!writeEmptyString()) return false;
        // Write format
        if (!writeTag(2, TagType::Byte4)) return false;
        if (!writeInt(2, &format)) return false;
        if (!writeSubBlockEnd(valueSubBlockStart)) return false;
    }
    return writeSubBlockEnd(subBlockStart);
}

bool TaggedBlockWriter::writeTextFormat(const TextFormat *textFormat) {
    if (!writeId(&textFormat->first)) return false;
    if (!writeId(&textFormat->second.timestamp)) return false;
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(2); subBlockStart == 0) return false;

    // Unknown byte that is always 17
    if (!writeByte(&writer::BYTE_SEVENTEEN)) return false;
    // Write legacy format
    const uint8_t format = static_cast<uint8_t>(textFormat->second.value.legacy);
    if (!writeByte(&format)) return false;
    if (textFormat->second.value.isLegacy) {
        const uint8_t baseStyle = static_cast<uint8_t>(textFormat->second.value.baseStyle);
        const uint32_t styleProperties = static_cast<uint32_t>(textFormat->second.value.styleProperties);

        if (!writeTag(2, TagType::Byte1)) return false;
        if (!writeByte(&baseStyle)) return false;
        if (!writeInt(3, &styleProperties)) return false;
    }
    return writeSubBlockEnd(subBlockStart);
}

uint32_t TaggedBlockWriter::writeBlockInfoHeaderStart(const BlockInfo &blockInfo) {
    const uint32_t startOffset = currentOffset;
    if (!writeBytes(sizeof(uint32_t), &writer::MAX_UINT32)) return 0;
    if (!writeBytes(sizeof(uint8_t), &writer::BYTE_ZERO)) return 0; // Skip 1 byte of 0x00
    uint8_t temp[3];
    temp[0] = blockInfo.minVersion;
    temp[1] = blockInfo.currentVersion;
    temp[2] = blockInfo.blockType;
    if (!writeBytes(3, temp)) return 0; // Write 3 bytes for block info

    return startOffset;
}

bool TaggedBlockWriter::writeBlockInfoHeaderEnd(const uint32_t blockStartOffset) {
    const uint32_t size = currentOffset - blockStartOffset - 8;
    const uint32_t returnOffset = currentOffset;
    seekTo(blockStartOffset);
    if (!writeBytes(sizeof(uint32_t), &size)) return false;
    seekTo(returnOffset);
    return true;
}


template<typename T>
uint32_t TaggedBlockWriter::_writeLwwItemId(const uint8_t index, const LwwItem<T> *id) {
    uint32_t subBlockStart;
    if (subBlockStart = writeSubBlockStart(index); subBlockStart == 0) return 0;
    if (!writeId(1, &id->timestamp)) return -1;
    return subBlockStart;
}
