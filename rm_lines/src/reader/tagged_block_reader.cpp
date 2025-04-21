// ReSharper disable CppTooWideScopeInitStatement
#include <reader/tagged_block_reader.h>
#include <cstring>
#include <format>
#include <iomanip>
#include <library.h>
#include <sstream>

TaggedBlockReader::~TaggedBlockReader() {
}


bool TaggedBlockReader::readBlockInfo() {
    tagClaimed = true; // Reset tag claimed status
    if (currentOffset + 8 > dataSize_) return false;
    // Ensure enough data (4 + 1 + 1 + 1)

    currentBlockInfo.offset = currentOffset + 8; // Store offset at block data start
    readBytesOrError(sizeof(uint32_t), &currentBlockInfo.size);

    skipBytes(1); // Skip 1 unknown byte

    uint8_t temp[3];
    readBytesOrError(3, temp); // Read 3 bytes for currentBlockInfo

    currentBlockInfo.minVersion = temp[0];
    currentBlockInfo.currentVersion = temp[1];
    currentBlockInfo.blockType = temp[2];

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
    currentBlock = Block::lookup(currentBlockInfo);
    return currentBlock ? currentBlock->read(this) : false;
}


bool TaggedBlockReader::readSubBlock(const uint8_t index, SubBlockInfo &subBlockInfo) {
    if (!checkRequiredTag(index, TagType::Length4)) return false;
    uint32_t subBlockLength;
    readBytesOrError(sizeof(uint32_t), &subBlockLength); // Read the length of the subblock
    subBlockInfo.offset = currentOffset;
    subBlockInfo.size = subBlockLength;
    return true;
}

bool TaggedBlockReader::readSubBlock(const uint8_t index) {
    if (!checkRequiredTag(index, TagType::Length4)) return false;
    skipBytes(4); // Skip the length
    return true;
}

bool TaggedBlockReader::checkSubBlock(const uint8_t index) {
    if (!hasBytesRemaining()) {
        return false;
    }
    const bool subBlockPresent = checkTag(index, TagType::Length4);
    return subBlockPresent;
}

bool TaggedBlockReader::readValuint(uint64_t &result) {
    result = 0;
    uint8_t shift = 0;

    while (currentOffset < dataSize_) {
        uint8_t byte;
        if (!readBytes(1, &byte)) return false; // Read a byte
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
    readBytesOrError(length, uuid_bytes.data()); // Read the uuid data

    // Convert bytes to hexadecimal string without modifying byte order (little-endian)
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(uuid_bytes[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-"; // Add dashes at appropriate places
    }

    uuid = ss.str();
    return true;
}

bool TaggedBlockReader::readBytes(const uint32_t size, void *result) {
    if (size == 0)
        return true; // nothing to do, skip.
    if (currentOffset + size > dataSize_) {
        logError(std::format("Attempted to read beyond the end of the data: {} - {} / {}", currentOffset,
                             currentOffset + size, dataSize_));
        return false;
    }

    // Read the bytes directly into the destination buffer
    if (fread(result, size, 1, file) != 1) {
        logError(std::format("Failed to read {} bytes, {} - {} / {}", size, currentOffset,
                             currentOffset + size, dataSize_));
        return false;
    }

    // Update the current offset
    currentOffset += size;

    return true;
}

void TaggedBlockReader::readBytesOrError(uint32_t size, void *result) {
    if (size == 0)
        return; // nothing to do, skip.
    if (currentOffset + size > dataSize_)
        throw std::runtime_error(std::format("Attempted to read beyond the end of the data: {} - {} / {}",
                                             currentOffset,
                                             currentOffset + size, dataSize_));

    // Read the bytes directly into the destination buffer
    if (fread(result, size, 1, file) != 1)
        throw std::runtime_error(std::format("Failed to read {} bytes, {} - {} / {}", size,
                                             currentOffset,
                                             currentOffset + size, dataSize_));

    // Update the current offset
    currentOffset += size;
}


void TaggedBlockReader::skipBytes(const uint32_t size) {
    const long result = fseek(file, size, SEEK_CUR);
    if (result == -1) {
        throw std::runtime_error(std::format("Failed to skip bytes {}", size));
    }
    currentOffset += size;
}

void TaggedBlockReader::seekTo(uint32_t offset) {
    if (offset > dataSize_) {
        throw std::out_of_range("Offset exceeds data size");
    }
    const long result = fseek(file, offset, SEEK_SET);

    if (result == -1) {
        throw std::runtime_error(std::format("Failed to seek to offset {}", offset));
    }

    currentOffset = offset;
}

bool TaggedBlockReader::checkTag(const uint8_t expectedIndex, const TagType expectedTagType) const {
    return tag.index == expectedIndex && tag.type == expectedTagType;
}

bool TaggedBlockReader::checkRequiredTag(const uint8_t expectedIndex, const TagType expectedTagType) {
    if (tag.index == expectedIndex && tag.type == expectedTagType) {
        claimTag(); // Claim the tag
        return true;
    } else {
        logError(std::format("Expected tag index {} and type {}, but got index {} and type {}", expectedIndex,
                             static_cast<int>(expectedTagType), tag.index, static_cast<int>(tag.type)));
        return false;
    }
}

void TaggedBlockReader::getTag() {
    if (!tagClaimed) {
        return; // Tag not claimed, no need to read it again
    }
    const auto [index, tagType] = _readTagValues();
    // Update temporary tag info
    tag.index = index;
    tag.type = tagType;
    if (tag.type != TagType::BAD_LENGTH && tag.type != TagType::BAD_VALUINT) {
        tagClaimed = false; // mark the tag as unclaimed
    }
    // logDebug(std::format("Read [TAG] index: {}, type: {}", tag.index, static_cast<int>(tag.type)));
}

void TaggedBlockReader::claimTag() {
    if (tagClaimed) {
        logError("Already claimed tag was attempted to be claimed again!");
        return;
    }
    tagClaimed = true;
}

std::pair<uint8_t, TagType> TaggedBlockReader::_readTagValues() {
    if (!remainingBytes()) return {0, TagType::BAD_LENGTH};
    uint64_t x;
    if (!readValuint(x)) return {0, TagType::BAD_VALUINT};

    uint8_t index = x >> 4;
    auto tagType = static_cast<TagType>(x & 0xF);
    return {index, tagType};
}

// Read the values

bool TaggedBlockReader::readId(const uint8_t index, CrdtId *id) {
    getTag();
    if (!checkRequiredTag(index, TagType::ID)) return false;
    return readId(id);
}

bool TaggedBlockReader::readId(CrdtId *id) {
    readBytesOrError(sizeof(id->first), &id->first);
    return readValuint(id->second);
}

bool TaggedBlockReader::readBool(const uint8_t index, bool *result) {
    getTag();
    if (!checkRequiredTag(index, TagType::Byte1)) return false;
    uint8_t byte;
    readBytesOrError(1, &byte);
    if (result == nullptr) return true; // If result is null, just return true
    *result = (byte != 0);
    return true;
}

bool TaggedBlockReader::readBool(bool *result) {
    if (currentOffset >= dataSize_) return false;
    uint8_t byte;
    readBytesOrError(1, &byte);
    if (result == nullptr) return true; // If result is null, just return true
    *result = (byte != 0);
    return true;
}

bool TaggedBlockReader::readInt(const uint8_t index, uint32_t *result) {
    getTag();
    if (!checkRequiredTag(index, TagType::Byte4)) return false;
    return readInt(result);
}

bool TaggedBlockReader::readInt(uint32_t *result) {
    return readBytes(sizeof(uint32_t), result);
}

bool TaggedBlockReader::readIntPair(uint8_t index, IntPair *result) {
    getTag();
    if (!readSubBlock(index)) return false;
    return readIntPair(result);
}

bool TaggedBlockReader::readIntPair(IntPair *result) {
    return readBytes(sizeof(IntPair), result);
}

bool TaggedBlockReader::readFloat(const uint8_t index, float *result) {
    getTag();
    if (!checkRequiredTag(index, TagType::Byte4)) return false;
    return readFloat(result);
}

bool TaggedBlockReader::readFloat(float *result) {
    return readBytes(sizeof(float), result);
}

bool TaggedBlockReader::readDouble(uint8_t index, double *result) {
    getTag();
    if (!checkRequiredTag(index, TagType::Byte8)) return false;
    return readDouble(result);
}

bool TaggedBlockReader::readDouble(double *result) {
    return readBytes(sizeof(double), result);
}

bool TaggedBlockReader::readByte(const uint8_t index, uint8_t *result) {
    getTag();
    if (!checkRequiredTag(index, TagType::Byte1)) return false;
    return readByte(result);
}

bool TaggedBlockReader::readByte(uint8_t *result) {
    return readBytes(sizeof(uint8_t), result);
}


bool TaggedBlockReader::readString(const uint8_t index, std::string *result) {
    getTag();
    if (!readSubBlock(index)) return false;

    return readString(result);
}

bool TaggedBlockReader::readString(std::string *result) {
    uint64_t stringLength;
    bool isAscii;
    if (!readValuint(stringLength)) return false;
    if (!readBool(&isAscii)) return false;
    if (!isAscii) return false;

    const auto stringData = std::make_unique<uint8_t[]>(stringLength);
    if (!readBytes(stringLength, stringData.get())) return false;

    // Assign the string data to the result
    result->assign(reinterpret_cast<char *>(stringData.get()), stringLength);
    return true;
}

bool TaggedBlockReader::readColor(uint8_t index, Color *result) {
    getTag();
    if (!checkRequiredTag(index, TagType::Byte4)) return false;
    return readColor(result);
}

bool TaggedBlockReader::readColor(Color *result) {
    return readBytes(4, result);
}

// Lww

bool TaggedBlockReader::readLwwId(const uint8_t index, LwwItem<CrdtId> *id) {
    if (!_readLwwItemId<CrdtId>(index, id)) return false;
    if (!readId(2, &id->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwBool(const uint8_t index, LwwItem<bool> *result) {
    if (!_readLwwItemId<bool>(index, result)) return false;
    if (!readBool(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwFloat(const uint8_t index, LwwItem<float> *result) {
    if (!_readLwwItemId<float>(index, result)) return false;
    if (!readFloat(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwByte(const uint8_t index, LwwItem<uint8_t> *result) {
    if (!_readLwwItemId<uint8_t>(index, result)) return false;
    if (!readByte(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwString(const uint8_t index, LwwItem<std::string> *result) {
    if (!_readLwwItemId<std::string>(index, result)) return false;
    if (!readString(2, &result->value)) return false;
    return true;
}

// Special

bool TaggedBlockReader::readStringWithFormat(uint8_t index, StringWithFormat *result) {
    if (!readSubBlock(index)) return false;
    if (!readString(&result->first)) return false;
    getTag();
    if (checkTag(2, TagType::Byte4)) {
        uint32_t _format;
        if (!readInt(2, &_format)) return false;
        result->second = _format;
    }
    return true;
}

bool TaggedBlockReader::readTextItem(TextItem *textItem) {
    getTag();
    if (!readSubBlock(0)) return false; // Read into text item subblock
    if (!readId(2, &textItem->itemId)) return false;
    if (!readId(3, &textItem->leftId)) return false;
    if (!readId(4, &textItem->rightId)) return false;
    if (!readInt(5, &textItem->deletedLength)) return false;

    getTag();
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

    getTag();
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
bool TaggedBlockReader::_readLwwItemId(const uint8_t index, LwwItem<T> *id) {
    getTag();
    if (!readSubBlock(index)) return false;
    if (!readId(1, &id->timestamp)) return false;
    return true;
}


bool TaggedBlockReader::buildTree(SceneTree &tree) {
    while (readBlockInfo()) {
        logDebug(std::format("Read block info header m{}/c{} OF: {} SB: {} BT: {}", currentBlockInfo.minVersion,
                             currentBlockInfo.currentVersion, currentBlockInfo.offset,
                             currentBlockInfo.size, currentBlockInfo.blockType));

        uint32_t block_end = currentBlockInfo.offset + currentBlockInfo.size;
        if (!readBlock()) {
            // Skip block
            seekTo(block_end);
            logError(std::format("Failed to read block type {}", currentBlockInfo.blockType));
            continue;
        } else if (currentOffset < block_end) {
            logError(std::format("BLOCK type: {} under-read: {} offset < {} block end", currentBlockInfo.blockType,
                                 currentOffset, block_end));
            seekTo(block_end);
        } else if (currentOffset > block_end) {
            logError(std::format("BLOCK type: {} over-read : {} offset > {} block end", currentBlockInfo.blockType,
                                 currentOffset, block_end));
            seekTo(block_end);
            continue;
        }

        // Handle the block into the tree
        switch (currentBlockInfo.getBlockType()) {
            case SCENE_TREE_BLOCK: {
                // Add the scene tree as a tree node
                const auto sceneTreeBlock = dynamic_cast<SceneTreeBlock *>(currentBlock.get());
                tree.addNode(sceneTreeBlock->treeId, sceneTreeBlock->parentId);
                break;
            }
            case TREE_NODE_BLOCK: {
                // Update the node with its information
                // This node should already exist

                const auto treeNodeBlock = dynamic_cast<TreeNodeBlock *>(currentBlock.get());
                Group *node = tree.getNode(treeNodeBlock->group.nodeId);

                node->label = treeNodeBlock->group.label;
                node->visible = treeNodeBlock->group.visible;
                node->anchorId = treeNodeBlock->group.anchorId;
                node->anchorType = treeNodeBlock->group.anchorType;
                node->anchorOriginX = treeNodeBlock->group.anchorOriginX;
                node->anchorThreshold = treeNodeBlock->group.anchorThreshold;
                break;
            }
            case SCENE_GROUP_ITEM_BLOCK: {
                // Add this node to its parent

                const auto groupItemBlock = dynamic_cast<SceneGroupItemBlock *>(currentBlock.get());
                groupItemBlock->item.applyTreeValue(tree, groupItemBlock->item.value.value());
                tree.addItem(groupItemBlock->item, groupItemBlock->parentId);
                break;
            }
            case SCENE_LINE_ITEM_BLOCK: {
                const auto lineItemBlock = dynamic_cast<SceneLineItemBlock *>(currentBlock.get());
                tree.addItem(lineItemBlock->item, lineItemBlock->parentId);
                break;
            }
            case SCENE_GLYPH_ITEM_BLOCK: {
                const auto glyphItemBlock = dynamic_cast<SceneGlyphItemBlock *>(currentBlock.get());
                tree.addItem(glyphItemBlock->item, glyphItemBlock->parentId);
                break;
            }
            case SCENE_INFO_BLOCK: {
                const auto sceneInfoBlock = dynamic_cast<SceneInfoBlock *>(currentBlock.get());
                tree.sceneInfo = *sceneInfoBlock;
                break;
            }
            case ROOT_TEXT_BLOCK: {
                const auto rootTextBlock = dynamic_cast<RootTextBlock *>(currentBlock.get());
                tree.rootText = rootTextBlock->value;
                break;
            }
            default: break;
        }
    }

    return false;
}
