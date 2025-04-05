#include <reader/tagged_block_reader.h>
#include <cstring>
#include <format>
#include <iomanip>
#include <library.h>
#include <sstream>

bool TaggedBlockReader::readBlockInfo() {
    if (currentOffset + 8 > dataSize_) return false;
    // Ensure enough data (4 + 1 + 1 + 1)

    currentBlockInfo.offset = currentOffset + 8; // Store offset at block data start
    std::memcpy(&currentBlockInfo.size, data_ + currentOffset, sizeof(uint32_t));
    currentOffset += 4; // Move past block_size

    currentOffset += 1; // Skip 'unknown'

    currentBlockInfo.minVersion = data_[currentOffset];
    currentBlockInfo.currentVersion = data_[currentOffset + 1];
    currentBlockInfo.blockType = data_[currentOffset + 2];

    currentOffset += 3; // Move forward by 3 bytes

    return true;
}

bool TaggedBlockReader::hasBytesRemaining() const {
    return remainingBytes() > 0;
}

uint32_t TaggedBlockReader::remainingBytes() const {
    const uint32_t blockEnd = currentBlockInfo.offset + currentBlockInfo.size;
    if (currentOffset >= blockEnd) {
        return 0;
    }
    return blockEnd - currentOffset;
}

bool TaggedBlockReader::readBlock() {
    Block::lookup(currentBlock, currentBlockInfo);
    return currentBlock != nullptr ? currentBlock->read(this) : false;
}


bool TaggedBlockReader::readSubBlock(const uint8_t index, SubBlockInfo &subBlockInfo) {
    if (!readRequiredTag(index, TagType::Length4)) return false;
    uint32_t subBlockLength;
    memcpy(&subBlockLength, data_ + currentOffset, sizeof(uint32_t));
    currentOffset += 4; // Move past subBlockLength
    subBlockInfo.offset = currentOffset;
    subBlockInfo.size = subBlockLength;
    return true;
}

bool TaggedBlockReader::readSubBlock(const uint8_t index) {
    if (!readRequiredTag(index, TagType::Length4)) return false;
    currentOffset += 4; // Move past subBlockLength
    return true;
}

bool TaggedBlockReader::checkSubBlock(const uint8_t index) {
    const uint32_t previousPosition = currentOffset;
    if (!hasBytesRemaining()) {
        return false;
    }
    const bool subBlockPresent = readTag(index, TagType::Length4);
    currentOffset = previousPosition;
    return subBlockPresent;
}

bool TaggedBlockReader::readValuint(uint64_t &result) {
    result = 0;
    uint8_t shift = 0;

    while (currentOffset < dataSize_) {
        uint8_t byte = data_[currentOffset++];
        result |= static_cast<uint64_t>(byte & 0x7F) << shift;
        if ((byte & 0x80) == 0) return true;
        shift += 7;
        if (shift >= 64) break; // Prevent overflow
    }
    return false; // Error: Truncated or invalid data
}

bool TaggedBlockReader::readUUID(std::string &uuid, const uint32_t length) {
    if (currentOffset + length > dataSize_) return false;

    // Read the UUID bytes (of variable length)
    std::vector<uint8_t> uuid_bytes(length);
    for (size_t i = 0; i < length; ++i) {
        uuid_bytes[i] = data_[currentOffset + i]; // Read data directly into the uuid_bytes vector
    }
    currentOffset += length; // Move the currentOffset forward by length (in bytes)

    // Convert bytes to hexadecimal string without modifying byte order (little-endian)
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(uuid_bytes[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-"; // Add dashes at appropriate places
    }

    uuid = ss.str();
    return true;
}

bool TaggedBlockReader::readBytes(uint32_t size, void *dest) {
    if (currentOffset + size > dataSize_) return false;

    // Read the bytes directly into the destination buffer
    std::memcpy(dest, &data_[currentOffset], size);

    // Update the current offset
    currentOffset += size;

    return true;
}

bool TaggedBlockReader::checkTag(uint8_t expectedIndex, TagType expectedTagType) {
    const uint32_t previousPosition = currentOffset;
    const auto [index, tagType] = _readTagValues();
    currentOffset = previousPosition;
    return index == expectedIndex && tagType == expectedTagType;
}

bool TaggedBlockReader::readTag(uint8_t expectedIndex, TagType expectedTagType) {
    const auto [index, tagType] = _readTagValues();
    return index == expectedIndex && tagType == expectedTagType;
}


bool TaggedBlockReader::readRequiredTag(const uint8_t expectedIndex, const TagType expectedTagType) {
    const auto [index, tagType] = _readTagValues();
    if (index == expectedIndex && tagType == expectedTagType) {
        return true;
    } else {
        logError(std::format("Expected tag index {} and type {}, but got index {} and type {}", expectedIndex,
                             static_cast<int>(expectedTagType), index, static_cast<int>(tagType)));
        return false;
    }
}

std::pair<uint8_t, TagType> TaggedBlockReader::_readTagValues() {
    uint64_t x;
    if (!readValuint(x)) return {0, TagType::BAD};

    uint8_t index = x >> 4;
    auto tagType = static_cast<TagType>(x & 0xF);
    return {index, tagType};
}

// Read the values

bool TaggedBlockReader::readId(const uint8_t index, CrdtId *id) {
    if (!readRequiredTag(index, TagType::ID)) return false;
    return readId(id);
}

bool TaggedBlockReader::readId(CrdtId *id) {
    id->first = data_[currentOffset++];
    return readValuint(id->second);
}

bool TaggedBlockReader::readBool(const uint8_t index, bool *result) {
    if (!readRequiredTag(index, TagType::Byte1)) return false;
    const uint8_t byte = data_[currentOffset++];
    if (result == nullptr) return true; // If result is null, just return true
    *result = (byte != 0);
    return true;
}

bool TaggedBlockReader::readBool(bool *result) {
    if (currentOffset >= dataSize_) return false;
    const uint8_t byte = data_[currentOffset++];
    if (result == nullptr) return true; // If result is null, just return true
    *result = (byte != 0);
    return true;
}

bool TaggedBlockReader::readInt(const uint8_t index, uint32_t *result) {
    if (!readRequiredTag(index, TagType::Byte4)) return false;
    return readInt(result);
}

bool TaggedBlockReader::readInt(uint32_t *result) {
    return readBytes(sizeof(uint32_t), result);
}

bool TaggedBlockReader::readIntPair(uint8_t index, IntPair *result) {
    if (!readSubBlock(index)) return false;
    return readIntPair(result);
}

bool TaggedBlockReader::readIntPair(IntPair *result) {
    return readBytes(sizeof(IntPair), result);
}

bool TaggedBlockReader::readFloat(const uint8_t index, float *result) {
    if (!readRequiredTag(index, TagType::Byte4)) return false;
    return readFloat(result);
}

bool TaggedBlockReader::readFloat(float *result) {
    return readBytes(sizeof(float), result);
}

bool TaggedBlockReader::readDouble(uint8_t index, double *result) {
    if (!readRequiredTag(index, TagType::Byte8)) return false;
    return readDouble(result);
}

bool TaggedBlockReader::readDouble(double *result) {
    return readBytes(sizeof(double), result);
}

bool TaggedBlockReader::readByte(const uint8_t index, uint8_t *result) {
    if (!readRequiredTag(index, TagType::Byte1)) return false;
    return readByte(result);
}

bool TaggedBlockReader::readByte(uint8_t *result) {
    return readBytes(sizeof(uint8_t), result);
}


bool TaggedBlockReader::readString(const uint8_t index, std::string *result) {
    if (!readSubBlock(index)) return false;

    return readString(result);
}

bool TaggedBlockReader::readString(std::string *result) {
    uint64_t stringLength;
    bool isAscii;
    if (!readValuint(stringLength)) return false;
    if (!readBool(&isAscii)) return false;

    auto stringData = new uint8_t[stringLength];
    if (!readBytes(stringLength, stringData)) {
        delete[] stringData;
        return false;
    }

    // Parse ascii or utf8
    if (isAscii) {
        result->assign(reinterpret_cast<char *>(stringData), stringLength);
    } else {
        // UTF-8 parsing
        const std::string utf8String(reinterpret_cast<char *>(stringData), stringLength);
        result->assign(utf8String);
    }
    delete[] stringData;
    return true;
}

bool TaggedBlockReader::readColor(uint8_t index, Color *result) {
    if (!readRequiredTag(index, TagType::Byte4)) return false;
    return readColor(result);
}

bool TaggedBlockReader::readColor(Color *result) {
    return readBytes(4, result);
}

// Lww

bool TaggedBlockReader::readLwwId(const uint8_t index, LwwItem<CrdtId> *id) {
    if (!_readLwwTimestamp<CrdtId>(index, id)) return false;
    if (!readId(2, &id->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwBool(const uint8_t index, LwwItem<bool> *result) {
    if (!_readLwwTimestamp<bool>(index, result)) return false;
    if (!readBool(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwFloat(const uint8_t index, LwwItem<float> *result) {
    if (!_readLwwTimestamp<float>(index, result)) return false;
    if (!readFloat(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwByte(const uint8_t index, LwwItem<uint8_t> *result) {
    if (!_readLwwTimestamp<uint8_t>(index, result)) return false;
    if (!readByte(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwString(const uint8_t index, LwwItem<std::string> *result) {
    if (!_readLwwTimestamp<std::string>(index, result)) return false;
    if (!readString(2, &result->value)) return false;
    return true;
}

// Special

bool TaggedBlockReader::readStringWithFormat(uint8_t index, StringWithFormat *result) {
    if (!readSubBlock(index)) return false;
    if (!readString(&result->first)) return false;
    const uint32_t previousOffset = currentOffset;
    if (checkTag(2, TagType::Byte4)) {
        uint32_t _format;
        if (!readInt(2, &_format)) return false;
        result->second = _format;
    } else {
        currentOffset = previousOffset;
    }
    return true;
}

bool TaggedBlockReader::readTextItem(TextItem *textItem) {
    if (!readSubBlock(0)) return false; // Read into text item subblock
    if (!readId(2, &textItem->itemId)) return false;
    if (!readId(3, &textItem->leftId)) return false;
    if (!readId(4, &textItem->rightId)) return false;
    if (!readInt(5, &textItem->deletedLength)) return false;

    if (checkSubBlock(6)) {
        StringWithFormat stringWithFormat;
        if (!readStringWithFormat(6, &stringWithFormat)) return false;

        if (stringWithFormat.second.has_value()) {
            textItem->value = stringWithFormat.second.value();
        } else {
            textItem->value = stringWithFormat.first;
        }
    } else {
        textItem->value = "";
    }
    return true;
}

bool TaggedBlockReader::readTextFormat(TextFormat *textFormat) {
    if (!readId(&textFormat->first)) return false;
    if (!readId(1, &textFormat->second.timestamp)) return false;

    if (!readSubBlock(2)) return false;

    uint8_t unknown;
    if (!readByte(&unknown)) return false;
    if (unknown != 17) return false;

    uint8_t format;
    if (!readByte(&format)) return false;


    textFormat->second.value = static_cast<ParagraphStyle>(format);

    return true;
}

template<typename T>
bool TaggedBlockReader::_readLwwTimestamp(const uint8_t index, LwwItem<T> *id) {
    if (!readSubBlock(index)) return false;
    if (!readId(1, &id->timestamp)) return false;
    return true;
}


bool TaggedBlockReader::buildTree(SceneTree *tree) {
    while (readBlockInfo()) {
        logMessage(std::format("Read block info header {}:{} OF: {} S: {} BT: {}", currentBlockInfo.minVersion,
                               currentBlockInfo.currentVersion, currentBlockInfo.offset,
                               currentBlockInfo.size, currentBlockInfo.blockType));

        uint32_t block_end = currentBlockInfo.offset + currentBlockInfo.size;
        if (!readBlock()) {
            // Skip block
            currentOffset = block_end;
            logError(std::format("Failed to read block type {}", currentBlockInfo.blockType));
            continue;
        } else if (currentOffset < block_end) {
            logError(std::format("BLOCK {} DID NOT FULLY READ {} < {}", currentBlockInfo.blockType,
                                 currentOffset, block_end));
            currentOffset = block_end;
        } else if (currentOffset > block_end) {
            logError(std::format("BLOCK {} OVER READ {} > {}", currentBlockInfo.blockType,
                                 currentOffset, block_end));
            currentOffset = block_end;
            continue;
        } else {
            logMessage(std::format("Read block {}", currentBlockInfo.blockType));
        }

        // Handle the block into the tree
    }
    return false;
}
