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

bool TaggedBlockReader::bytesRemainingInBlock() const {
     return currentOffset < currentBlockInfo.offset + currentBlockInfo.size;
}

bool TaggedBlockReader::readBlock() {
    Block::lookup(currentBlock, currentBlockInfo);
    return currentBlock != nullptr ? currentBlock->read(this) : false;
}


bool TaggedBlockReader::readSubBlock(const uint8_t index, SubBlockInfo &subBlockInfo) {
    if (!readTag(index, TagType::Length4)) return false;
    uint32_t subBlockLength;
    memcpy(&subBlockLength, data_ + currentOffset, sizeof(uint32_t));
    currentOffset += 4; // Move past subBlockLength
    subBlockInfo.offset = currentOffset;
    subBlockInfo.size = subBlockLength;
    return true;
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

bool TaggedBlockReader::readUUID(std::string& uuid, const uint32_t length) {
    if (currentOffset + length > dataSize_) return false;

    // Read the UUID bytes (of variable length)
    std::vector<uint8_t> uuid_bytes(length);
    for (size_t i = 0; i < length; ++i) {
        uuid_bytes[i] = data_[currentOffset + i];  // Read data directly into the uuid_bytes vector
    }
    currentOffset += length;  // Move the currentOffset forward by length (in bytes)

    // Convert bytes to hexadecimal string without modifying byte order (little-endian)
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(uuid_bytes[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-"; // Add dashes at appropriate places
    }

    uuid = ss.str();
    return true;

}

bool TaggedBlockReader::readBytes(size_t size, void *dest) {
    if (currentOffset + size > dataSize_) return false;

    // Read the bytes directly into the destination buffer
    std::memcpy(dest, &data_[currentOffset], size);

    // Update the current offset
    currentOffset += size;

    return true;
}


bool TaggedBlockReader::readTag(const uint8_t expectedIndex, const TagType expectedTagType) {
    const auto [index, tagType] = _readTagValues();
    return index == expectedIndex && tagType == expectedTagType;
}

std::pair<uint8_t, TagType> TaggedBlockReader::_readTagValues() {
    uint64_t x;
    if (!readValuint(x)) return {0, TagType::BAD};

    uint8_t index = x >> 4;
    auto tagType = static_cast<TagType>(x & 0xF);
    return {index, tagType};
}

bool TaggedBlockReader::readId(const uint8_t index, CrdtId *id) {
    if (!readTag(index, TagType::ID)) return false;
    return _readCrdtId(id);
}

bool TaggedBlockReader::readBool(const uint8_t index, bool *result) {
    if (!readTag(index, TagType::Byte1)) return false;
    const uint8_t byte = data_[currentOffset++];
    if (result == nullptr) return true; // If result is null, just return true
    *result = (byte != 0);
    return true;
}

bool TaggedBlockReader::readInt(const uint8_t index, uint32_t *result) {
    if (!readTag(index, TagType::Byte4)) return false;
    return readBytes(sizeof(uint32_t), result);
}

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

bool TaggedBlockReader::readLwwIntPair(const uint8_t index, LwwItem<IntPair> *result) {
    if (!_readLwwTimestamp<IntPair>(index, result)) return false;
    if (!readBytes(sizeof(IntPair), &result->value)) return false;
    return true;
}





bool TaggedBlockReader::_readCrdtId(CrdtId *id) {
    id->first = data_[currentOffset++];
    return readValuint(id->second);
}

template<typename T>
bool TaggedBlockReader::_readLwwTimestamp(const uint8_t index, LwwItem<T> *id) {
    if (SubBlockInfo subBlockInfo; !readSubBlock(index, subBlockInfo)) return false;
    if (!readId(1, &id->timestamp)) return false;
    return true;
}


bool TaggedBlockReader::buildTree() {
    return false;
}
