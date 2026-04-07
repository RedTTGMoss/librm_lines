// ReSharper disable CppTooWideScopeInitStatement
#include <reader/tagged_block_reader.h>
#include <reader/analyzer.h>
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

    skipBytes(1); // Skip 1 unknown byte of 0x00

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
    currentBlock->info = currentBlockInfo;
    // if (currentBlockInfo.blockType == AUTHOR_IDS_BLOCK) {
    //     return false;
    // }
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
    skipBytes(4);
    return true;
}

bool TaggedBlockReader::checkSubBlock(const uint8_t index) const {
    if (!hasBytesRemaining()) {
        return false;
    }
    const bool subBlockPresent = checkTag(index, TagType::Length4);
    return subBlockPresent;
}

bool TaggedBlockReader::debugSubBlock(uint8_t index) {
    SubBlockInfo subBlockInfo;
    if (!checkSubBlock(index)) {
        logDebug(std::format("SubBlock with index {} not present at offset {}. Remaining bytes: {}", index,
                             currentOffset,
                             remainingBytes()));
        return false;
    }
    if (!readSubBlock(index, subBlockInfo)) {
        return false;
    }
    logDebug(std::format("SubBlock with index {} - offset: {}, size: {}, currentOffset: {}, remainingBytes: {}",
                         index, subBlockInfo.offset, subBlockInfo.size, currentOffset, remainingBytes()));
    return true;
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

bool TaggedBlockReader::readUUID(const uint8_t index, std::string &uuid, const uint32_t length) {
    getTag();
    if (!readSubBlock(index)) return false;
    return readUUID(uuid, length);
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

bool TaggedBlockReader::checkRequiredTag(const uint8_t expectedIndex, const TagType expectedTagType, bool error) {
    if (tag.index == expectedIndex && tag.type == expectedTagType) [[likely]] {
        claimTag(); // Claim the tag
        return true;
    }
    if (error) {
        logError(std::format("Expected tag index {} and type 0x{:X} <{}>, but got index {} and type 0x{:X} <{}>",
                             expectedIndex,
                             static_cast<uint8_t>(expectedTagType), debugTagTypes(expectedTagType), tag.index,
                             static_cast<uint8_t>(tag.type), debugTagTypes(tag.type)));
    }
    return false;
}

void TaggedBlockReader::getTag() {
    if (!tagClaimed) {
        return; // Tag not claimed, no need to read it again
    }
    const int offsetPreTag = currentOffset;
    const auto [index, tagType] = _readTagValues();
    // Update temporary tag info
    tag.index = index;
    tag.type = tagType;


    if (tag.type != TagType::BAD_LENGTH && tag.type != TagType::BAD_VALUINT) {
        tagClaimed = false; // mark the tag as unclaimed
        if (tag.type == TagType::MisreadValuint) [[unlikely]] {
            // Debug misread valuint
            seekTo(offsetPreTag);
            uint64_t result;
            readValuint(result);
            seekTo(offsetPreTag);
            // Go back to before the tag if any other code wants to attempt reading this valuint properly
            logError(std::format("Misread Valuint tag encountered at offset {}. Value of valuint: {}", offsetPreTag,
                                 result));
        }
    }
    // logDebug(std::format("Read [TAG] index: {}, type: {}", tag.index, static_cast<int>(tag.type)));
}

bool TaggedBlockReader::debugTag(const uint8_t padding) {
    if (!tagClaimed) {
        return false;
    }
    std::string paddingStr(padding * 4, ' ');
    getTag();
    if (tag.type == TagType::BAD_LENGTH || tag.type == TagType::BAD_VALUINT) {
        return debugTag();
    }
    logDebug(std::format(
        "{}Read [TAG] index: {}, type: 0x{:X} <{}> offset: {}",
        paddingStr,
        tag.index,
        static_cast<unsigned int>(tag.type),
        debugTagTypes(tag.type),
        currentOffset
    ));
    if (tag.type == TagType::Length4 && checkSubBlock(tag.index)) {
        SubBlockInfo subBlockInfo;
        const bool success = readSubBlock(tag.index, subBlockInfo);
        if (!success) {
            logError(std::format("{}Failed to read subblock for tag index {}. Current offset: {}, data size: {}",
                                 paddingStr, tag.index, currentOffset, dataSize_));
            return false;
        }

        logDebug(std::format(
            "{}-- SubBlockInfo - offset: {}, size: {}",
            paddingStr, subBlockInfo.offset, subBlockInfo.size
        ));
        while (hasBytesRemaining() && currentOffset < subBlockInfo.offset + subBlockInfo.size) {
            if (!debugTag(padding + 1)) {
                break;
            }
        }
    }
    return true;
}

void TaggedBlockReader::claimTag() {
    if (tagClaimed) {
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


bool TaggedBlockReader::readInt(int32_t *result) {
    return readBytes(sizeof(int32_t), result);
}

bool TaggedBlockReader::readIntPair(const uint8_t index, IntPair *result) {
    getTag();
    if (!readSubBlock(index)) return false;
    return readIntPair(result);
}

bool TaggedBlockReader::readIntPair(IntPair *result) {
    return readBytes(sizeof(IntPair), result);
}

bool TaggedBlockReader::readDoublePair(const uint8_t index, DoublePair *result) {
    getTag();
    if (!readSubBlock(index)) return false;
    return readDoublePair(result);
}

bool TaggedBlockReader::readDoublePair(DoublePair *result) {
    return readBytes(sizeof(DoublePair), result);
}

bool TaggedBlockReader::readRectPair(uint8_t index, RectPair *result) {
    getTag();
    if (!readSubBlock(index)) return false;
    return readRectPair(result);
}

bool TaggedBlockReader::readRectPair(RectPair *result) {
    return readBytes(sizeof(RectPair), result);
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

bool TaggedBlockReader::readByteSub(const uint8_t index, uint8_t *result) {
    getTag();
    if (!readSubBlock(index)) return false;
    return readByte(result);
}

bool TaggedBlockReader::readBytesIndexed(const uint8_t index, const uint32_t size, std::vector<uint8_t> *result) {
    getTag();
    if (!checkRequiredTag(index, TagType::Byte1)) return false;
    return readBytes(size, result->data());
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

    if (stringLength > remainingBytes()) {
        return false;
    }

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
    uint32_t rawColor;
    if (!readBytes(sizeof(uint32_t), &rawColor)) return false;
    result->inplaceFromRGBA(&rawColor);
    return true;
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

bool TaggedBlockReader::readLwwByteSub(uint8_t index, LwwItem<uint8_t> *result) {
    if (!_readLwwItemId<uint8_t>(index, result)) return false;
    if (!readByteSub(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwDoublePair(const uint8_t index, LwwItem<DoublePair> *result) {
    if (!_readLwwItemId<DoublePair>(index, result)) return false;
    if (!readDoublePair(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwRectPair(uint8_t index, LwwItem<RectPair> *result) {
    if (!_readLwwItemId<RectPair>(index, result)) return false;
    if (!readRectPair(2, &result->value)) return false;
    return true;
}

bool TaggedBlockReader::readLwwUUID(const uint8_t index, LwwItem<std::string> *result) {
    if (!_readLwwItemId<std::string>(index, result)) return false;
    if (!readUUID(2, result->value, 16)) return false;
    return true;
}

bool TaggedBlockReader::readLwwBytes(const uint8_t index, LwwItem<std::vector<uint8_t> > *result) {
    const auto info = _readLwwItemIdInfo<std::vector<uint8_t> >(index, result);
    if (!info.has_value()) return false;
    SubBlockInfo subBlock;
    getTag();
    if (!readSubBlock(2, subBlock)) return false;
    result->value.resize(subBlock.size);
    if (!readBytes(subBlock.size, result->value.data())) return false;
    return true;
}

bool TaggedBlockReader::readLwwString(const uint8_t index, LwwItem<std::string> *result) {
    if (!_readLwwItemId<std::string>(index, result)) return false;
    if (!readString(2, &result->value)) return false;
    return true;
}

// Special

bool TaggedBlockReader::readStringWithFormat(const uint8_t index, StringWithFormat *result) {
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

    // Unknown byte
    uint8_t unknown;
    if (!readByte(&unknown)) return false;
    if (unknown != 17) return false;

    uint8_t format;
    if (!readByte(&format)) return false;

    textFormat->second.value.legacy = static_cast<ParagraphStyle>(format);

    // Try to read new formatting styles
    const int offset = currentOffset;
    getTag();
    if (checkRequiredTag(2, TagType::Byte1, false)) {
        readByte(&textFormat->second.value.baseStyle);
        claimTag();
        readInt(3, &textFormat->second.value.styleProperties);
    } else {
        // Rewind this check.
        claimTag();
        seekTo(offset);
        textFormat->second.value.baseStyle = 2;
        textFormat->second.value.styleProperties = format;
        textFormat->second.value.isLegacy = true;
    }

    return true;
}

template<typename T>
bool TaggedBlockReader::_readLwwItemId(const uint8_t index, LwwItem<T> *id) {
    getTag();
    if (!readSubBlock(index)) return false;
    if (!readId(1, &id->timestamp)) return false;
    return true;
}

template<typename T>
std::optional<SubBlockInfo> TaggedBlockReader::_readLwwItemIdInfo(const uint8_t index, LwwItem<T> *id) {
    SubBlockInfo info;
    getTag();
    if (!readSubBlock(index, info)) return std::nullopt;
    if (!readId(1, &id->timestamp)) return std::nullopt;
    return info;
}


bool TaggedBlockReader::buildTree(SceneTree &tree) {
    while (readBlockInfo()) {
        logDebug(std::format("Read block info header m{}/c{} OF: {} SB: {} BT: {}", currentBlockInfo.minVersion,
                             currentBlockInfo.currentVersion, currentBlockInfo.offset,
                             currentBlockInfo.size, currentBlockInfo.blockType));

        uint32_t block_end = currentBlockInfo.offset + currentBlockInfo.size;
        if (!readBlock() || currentOffset < block_end) {
            // Skip block

            logError(std::format("Failed to read block type {} (0x{:X})", currentBlockInfo.blockType,
                                 currentBlockInfo.blockType));
            if (currentOffset < block_end) {
                logError(std::format("BLOCK type: {} (0x{:X}) under-read: {} offset < {} block end",
                                     currentBlockInfo.blockType,
                                     currentBlockInfo.blockType,
                                     currentOffset, block_end));

                if (getDebugMode()) {
                    // Analyze first then seek back to dump
                    const int offset = currentOffset;
                    Analyzer::analyze(this);
                    seekTo(offset);

                    // Dump the remaining bytes for debugging
                    const uint32_t remaining = block_end - currentOffset;
                    std::vector<uint8_t> dumpData(remaining);
                    readBytesOrError(remaining, dumpData.data());
                    std::ostringstream oss;
                    oss << "Remaining bytes dump (hex): ";
                    for (const uint8_t byte: dumpData) {
                        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                    }
                    logDebug(oss.str());
                }
            } else {
                // Only analyze
                Analyzer::analyze(this);
            }
            seekTo(block_end);
        } else if (currentOffset > block_end) {
            logError(std::format("BLOCK type: {} (0x{:X}) over-read : {} offset > {} block end",
                                 currentBlockInfo.blockType,
                                 currentBlockInfo.blockType,
                                 currentOffset, block_end));
            seekTo(block_end);
            continue;
        }

        // Handle the block into the tree
        switch (currentBlockInfo.getBlockType()) {
            case AUTHOR_IDS_BLOCK: {
                const auto authorIdsBlock = dynamic_cast<AuthorIdsBlock *>(currentBlock.get());
                tree.authorsInfo = *authorIdsBlock;
                break;
            }
            case MIGRATION_INFO_BLOCK: {
                const auto migrationInfoBlock = dynamic_cast<MigrationInfoBlock *>(currentBlock.get());
                tree.migrationInfo = *migrationInfoBlock;
                break;
            }
            case PAGE_INFO_BLOCK: {
                const auto pageInfoBlock = dynamic_cast<PageInfoBlock *>(currentBlock.get());
                tree.pageInfo = *pageInfoBlock;
                break;
            }
            case SCENE_TREE_BLOCK: {
                // Add the scene tree as a tree node
                const auto sceneTreeBlock = dynamic_cast<SceneTreeBlock *>(currentBlock.get());
                tree.addNode(sceneTreeBlock->treeId, sceneTreeBlock->parentNodeId, sceneTreeBlock->parentTreeId);
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
                if (groupItemBlock->item.value.has_value()) {
                    groupItemBlock->item.applyTreeValue(tree, groupItemBlock->item.value.value());
                }
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
            case SCENE_IMAGE_ITEM_BLOCK: {
                const auto imageItemBlock = dynamic_cast<SceneImageItemBlock *>(currentBlock.get());
                tree.addItem(imageItemBlock->item, imageItemBlock->parentId);
                break;
            }
            case SCENE_INFO_BLOCK: {
                const auto sceneInfoBlock = dynamic_cast<SceneInfoBlock *>(currentBlock.get());
                tree.sceneInfo = *sceneInfoBlock;
                break;
            }
            case IMAGE_INFO_BLOCK: {
                const auto imageInfoBlock = dynamic_cast<ImageInfoBlock *>(currentBlock.get());
                tree.imageInfo = *imageInfoBlock;
                break;
            }
            case ROOT_TEXT_BLOCK: {
                const auto rootTextBlock = dynamic_cast<RootTextBlock *>(currentBlock.get());
                tree.rootText = rootTextBlock->value;
                break;
            }
            default:
                // Scold the devs for forgetting something UwU
                logDebug(std::format(
                    "!!!! No tree insertion for block type {}? Are you sure there isn't unfinished work? !!!!",
                    currentBlockInfo.blockType));
                break;
        }
    }

    return false;
}
