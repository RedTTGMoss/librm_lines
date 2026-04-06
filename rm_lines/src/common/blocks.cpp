// ReSharper disable CppDFAUnusedValue
#include <cstring>
#include <format>
#include <memory>
#include <library.h>
#include <common/blocks.h>
#include <reader/tagged_block_reader.h>
#include <common/scene_items.h>

#include "writer/tagged_block_writer.h"

BlockTypes BlockInfo::getBlockType() const {
    return static_cast<BlockTypes>(blockType);
}


std::unique_ptr<Block> Block::lookup(const BlockInfo &info) {
    std::unique_ptr<Block> block;
    switch (info.getBlockType()) {
        case MIGRATION_INFO_BLOCK:
            block = std::make_unique<MigrationInfoBlock>();
            break;
        case SCENE_TREE_BLOCK:
            block = std::make_unique<SceneTreeBlock>();
            break;
        case TREE_NODE_BLOCK:
            block = std::make_unique<TreeNodeBlock>();
            break;
        case SCENE_GLYPH_ITEM_BLOCK:
            block = std::make_unique<SceneGlyphItemBlock>();
            break;
        case SCENE_GROUP_ITEM_BLOCK:
            block = std::make_unique<SceneGroupItemBlock>();
            break;
        case SCENE_LINE_ITEM_BLOCK:
            block = std::make_unique<SceneLineItemBlock>();
            break;
        // case SCENE_TEXT_ITEM_BLOCK:  // TODO
        //     block = std::make_unique<SceneTextItemBlock>();
        //     break;
        case ROOT_TEXT_BLOCK:
            block = std::make_unique<RootTextBlock>();
            break;
        case SCENE_TOMBSTONE_ITEM_BLOCK: // TODO
            block = std::make_unique<SceneTombstoneItemBlock>();
            break;
        case AUTHOR_IDS_BLOCK:
            block = std::make_unique<AuthorIdsBlock>();
            break;
        case PAGE_INFO_BLOCK:
            block = std::make_unique<PageInfoBlock>();
            break;
        case SCENE_INFO_BLOCK:
            block = std::make_unique<SceneInfoBlock>();
            break;
        case IMAGE_INFO_BLOCK:
            block = std::make_unique<ImageInfoBlock>();
            break;
        case SCENE_IMAGE_ITEM_BLOCK:
            block = std::make_unique<SceneImageItemBlock>();
            break;
        default:
            block = std::make_unique<UnreadableBlock>();
            break;
    }
    return block;
}

Block::Block() {
}

Block::~Block() {
}


bool Block::read(TaggedBlockReader *reader) {
    return false;
}

bool Block::write(TaggedBlockWriter *writer) const {
    return false;
}

BlockTypes Block::getBlockType() const {
    return UNREADABLE_BLOCK;
}

bool SceneTombstoneItemBlock::read(TaggedBlockReader *reader) {
    // TODO: Figure out tombstones
    // These seem to be related to deleted paragraphs
    CrdtId _item1;
    CrdtId _posX;
    CrdtId _posY;
    CrdtId _item4;
    uint32_t integer;
    if (!reader->readId(1, &_item1)) return false;

    if (!reader->readId(2, &_posX)) return false;
    if (!reader->readId(3, &_posY)) return false;
    if (!reader->readId(4, &_item4)) return false;
    if (!reader->readInt(5, &integer)) return false;

    logDebug(std::format("      The item1 is ({}:{})", _item1.first, _item1.second));
    logDebug(std::format("      The endId is ({}:{})", _posX.first, _posX.second));
    logDebug(std::format("      The startId is ({}:{})", _posY.first, _posY.second));
    logDebug(std::format("      The item4 is ({}:{})", _item4.first, _item4.second));
    logDebug(std::format("      The integer is {}", integer));
    return true;
}

json SceneTombstoneItemBlock::toJson() const {
    // TODO: Implement this function
    return {};
}


bool AuthorIdsBlock::read(TaggedBlockReader *reader) {
    uint64_t subBlocks = 0;
    if (!reader->readValuint(subBlocks)) return false;

    for (uint64_t i = 0; i < subBlocks; i++) {
        // Read into subblock
        reader->getTag();
        if (!reader->readSubBlock(0)) return false;

        uint64_t uuidLength = 0;
        if (!reader->readValuint(uuidLength)) return false;

        std::string uuid;
        if (!reader->readUUID(uuid, uuidLength)) return false;

        uint16_t authorId = 0;
        if (!reader->readBytes(sizeof(authorId), &authorId)) return false;

        authorIds[authorId] = uuid;
    }

    return true;
}

bool AuthorIdsBlock::write(TaggedBlockWriter *writer) const {
    const uint64_t subBlocks = authorIds.size();
    if (!writer->writeValuint(subBlocks)) return false;

    constexpr uint64_t uuidLength = 16;
    for (const auto &[authorId, uuid]: authorIds) {
        int subBlockStart;
        if (subBlockStart = writer->writeSubBlockStart(0); subBlockStart < 0) return false;

        if (!writer->writeValuint(uuidLength)) return false;

        if (!writer->writeUUID(uuid)) return false;

        if (!writer->writeBytes(sizeof(uint16_t), &authorId)) return false;

        if (!writer->writeSubBlockEnd(subBlockStart)) return false;
    }
    return true;
}

json AuthorIdsBlock::toJson() const {
    json j = json::array();
    for (const auto &[index, uuid]: authorIds) {
        j.push_back({{"first", uuid}, {"second", index}});
    }
    return j;
}

bool MigrationInfoBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &migrationId)) return false;
    if (!reader->readBool(2, &isDevice)) return false;
    if (reader->hasBytesRemaining()) {
        bool _isV3;
        if (!reader->readBool(3, &_isV3)) return false;
        isV3 = _isV3;
    }
    return true;
}

bool MigrationInfoBlock::write(TaggedBlockWriter *writer) const {
    if (!writer->writeId(1, &migrationId)) return false;
    if (!writer->writeBool(2, isDevice)) return false;
    if (isV3.has_value()) {
        if (!writer->writeBool(3, isV3.value())) return false;
    }
    return true;
}

json MigrationInfoBlock::toJson() const {
    return {
        {"migrationId", migrationId.toJson()},
        {"isDevice", isDevice},
        {"isV3", isV3.has_value() ? json(isV3.value()) : nullptr}
    };
}

bool PageInfoBlock::read(TaggedBlockReader *reader) {
    if (!reader->readInt(1, &loadsCount)) return false;
    if (!reader->readInt(2, &mergesCount)) return false;
    if (!reader->readInt(3, &textCharsCount)) return false;
    if (!reader->readInt(4, &textLinesCount)) return false;
    if (reader->hasBytesRemaining()) {
        // Read typeFolioUseCount
        if (!reader->readInt(5, &typeFolioUseCount)) return false;
    }
    return true;
}

bool PageInfoBlock::write(TaggedBlockWriter *writer) const {
    if (!writer->writeInt(1, &loadsCount)) return false;
    if (!writer->writeInt(2, &mergesCount)) return false;
    if (!writer->writeInt(3, &textCharsCount)) return false;
    if (!writer->writeInt(4, &textLinesCount)) return false;
    if (!writer->writeInt(5, &typeFolioUseCount)) return false;
    return true;
}

json PageInfoBlock::toJson() const {
    return {
        {"loadsCount", loadsCount},
        {"mergesCount", mergesCount},
        {"textCharsCount", textCharsCount},
        {"textLinesCount", textLinesCount},
        {"typeFolioUseCount", typeFolioUseCount}
    };
}

bool SceneInfoBlock::read(TaggedBlockReader *reader) {
    if (!reader->readLwwId(1, &currentLayer)) return false;

    if (reader->hasBytesRemaining()) {
        LwwItem<bool> _backgroundVisible;
        if (!reader->readLwwBool(2, &_backgroundVisible)) return false;
        backgroundVisible = _backgroundVisible;
    }
    if (reader->hasBytesRemaining()) {
        LwwItem<bool> _rootDocumentVisible;
        if (!reader->readLwwBool(3, &_rootDocumentVisible)) return false;
        rootDocumentVisible = _rootDocumentVisible;
    }
    if (reader->hasBytesRemaining()) {
        IntPair _paperSize;
        if (!reader->readIntPair(5, &_paperSize)) return false;
        paperSize = _paperSize;
    }
    if (reader->hasBytesRemaining()) {
        LwwItem<RectPair> _extendedContentRect;
        if (!reader->readLwwRectPair(6, &_extendedContentRect)) return false;
        extendedContentRect = _extendedContentRect;
    }
    if (reader->hasBytesRemaining()) {
        DoublePair _paperSizeF;
        if (!reader->readDoublePair(7, &_paperSizeF)) return false;
        paperSizeF = _paperSizeF;
    }
    if (reader->hasBytesRemaining()) [[unlikely]] {
        LwwItem<DoublePair> _lwwPaperSizeF;
        if (!reader->readLwwDoublePair(8, &_lwwPaperSizeF)) return false;
        lwwPaperSizeF = _lwwPaperSizeF;
    }
    if (reader->hasBytesRemaining()) {
        LwwItem<uint8_t> _preferredLayout;
        if (!reader->readLwwByte(9, &_preferredLayout)) return false;
        preferredLayout = _preferredLayout;
    }

    return true;
}

bool SceneInfoBlock::write(TaggedBlockWriter *writer) const {
    if (!writer->writeLwwId(1, &currentLayer)) return false;

    if (backgroundVisible.has_value()) {
        if (!writer->writeLwwBool(2, &backgroundVisible.value())) return false;
    }
    if (rootDocumentVisible.has_value()) {
        if (!writer->writeLwwBool(3, &rootDocumentVisible.value())) return false;
    }
    if (paperSize.has_value()) {
        if (!writer->writeIntPair(5, &paperSize.value())) return false;
    }
    if (extendedContentRect.has_value()) {
        if (!writer->writeLwwRectPair(6, &extendedContentRect.value())) return false;
    }
    if (paperSizeF.has_value()) {
        if (!writer->writeDoublePair(7, &paperSizeF.value())) return false;
    }
    if (lwwPaperSizeF.has_value()) {
        if (!writer->writeLwwDoublePair(8, &lwwPaperSizeF.value())) return false;
    }
    if (preferredLayout.has_value()) {
        if (!writer->writeLwwByte(9, &preferredLayout.value())) return false;
    }

    return true;
}

json SceneInfoBlock::toJson() const {
    json j;
    j["currentLayer"] = currentLayer.value.toJson();
    if (backgroundVisible) {
        j["backgroundVisible"] = backgroundVisible->toJson();
    } else {
        j["backgroundVisible"] = nullptr;
    }
    if (rootDocumentVisible) {
        j["rootDocumentVisible"] = rootDocumentVisible->toJson();
    } else {
        j["rootDocumentVisible"] = nullptr;
    }
    if (paperSize) {
        j["paperSize"] = {paperSize->first, paperSize->second};
    } else {
        j["paperSize"] = nullptr;
    }
    if (extendedContentRect) {
        j["extendedContentRect"] = extendedContentRect->toJson();
    } else {
        j["extendedContentRect"] = nullptr;
    }
    if (paperSizeF) {
        j["paperSizeF"] = {paperSizeF->first, paperSizeF->second};
    } else {
        j["paperSizeF"] = nullptr;
    }
    if (lwwPaperSizeF) {
        j["lwwPaperSizeF"] = lwwPaperSizeF->toJson();
    } else {
        j["lwwPaperSizeF"] = nullptr;
    }
    if (preferredLayout) {
        j["preferredLayout"] = preferredLayout->toJson();
    } else {
        j["preferredLayout"] = nullptr;
    }
    return j;
}

bool SceneTreeBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &treeId)) return false;
    if (!reader->readId(2, &nodeId)) return false;
    if (!reader->readBool(3, &isUpdate)) return false;
    reader->getTag();
    if (!reader->readSubBlock(4)) return false;
    if (!reader->readId(1, &parentId)) return false;

    return true;
}

json SceneTreeBlock::toJson() const {
    // TODO: Implement this function
    return {};
}

bool TreeNodeBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &group.nodeId)) return false;
    if (!reader->readLwwString(2, &group.label)) return false;
    if (!reader->readLwwBool(3, &group.visible)) return false;
    if (reader->hasBytesRemaining()) {
        LwwItem<CrdtId> _anchorId;
        if (!reader->readLwwId(7, &_anchorId)) return false;
        group.anchorId = _anchorId;
        LwwItem<uint8_t> _anchorType;
        if (!reader->readLwwByte(8, &_anchorType)) return false;
        group.anchorType = _anchorType;
        LwwItem<float> _anchorThreshold;
        if (!reader->readLwwFloat(9, &_anchorThreshold)) return false;
        group.anchorThreshold = _anchorThreshold;
        LwwItem<float> _anchorOriginX;
        if (!reader->readLwwFloat(10, &_anchorOriginX)) return false;
        group.anchorOriginX = _anchorOriginX;
    }

    return true;
}

json TreeNodeBlock::toJson() const {
    // TODO: Implement this function
    return {};
}

bool SceneItemBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &parentId)) return false;
    if (!reader->readId(2, &item.itemId)) return false;
    if (!reader->readId(3, &item.leftId)) return false;
    if (!reader->readId(4, &item.rightId)) return false;
    if (!reader->readInt(5, &item.deletedLength)) return false;
    reader->getTag();
    if (reader->checkSubBlock(6)) {
        if (!reader->readSubBlock(6)) return false;
        uint8_t itemType;
        if (!reader->readBytes(sizeof(itemType), &itemType)) return false;
        if (itemType != _itemType) {
            logError(std::format("Item type mismatch: {} != {}", itemType, _itemType));
            return false;
        }
        return readValue(reader);
    }
    return true;
}

bool SceneGroupItemBlock::readValue(TaggedBlockReader *reader) {
    CrdtId _value;
    if (!reader->readId(2, &_value)) return false;
    item.value = _value;
    return true;
}

json SceneGroupItemBlock::toJson() const {
    // TODO: Implement this function
    return {};
}

bool SceneLineItemBlock::readValue(TaggedBlockReader *reader) {
    version = reader->currentBlockInfo.currentVersion;

    item.value = std::make_optional<Line>();
    return item.value.value().read(reader, version);
}

json SceneLineItemBlock::toJson() const {
    // TODO: Implement this function
    return {};
}

bool RootTextBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &blockId)) return false;
    if (blockId != CrdtId(0, 0)) return false;
    reader->getTag();
    if (!reader->readSubBlock(2)) return false; // Section one

    for (int i = 1; i <= 2; i++) {
        reader->getTag();
        if (!reader->readSubBlock(1)) return false; // Text items
    }

    uint64_t numberOfItems;
    if (!reader->readValuint(numberOfItems)) return false;


    for (uint64_t i = 0; i < numberOfItems; i++) {
        TextItem textItem;
        if (!reader->readTextItem(&textItem)) return false;
        value.items.add(std::move(textItem));
    }

    for (int i = 2; i >= 1; i--) {
        reader->getTag();
        if (!reader->readSubBlock(i)) return false; // Formatting
    }

    if (!reader->readValuint(numberOfItems)) return false;


    value.styles.resize(numberOfItems);

    for (uint64_t i = 0; i < numberOfItems; i++) {
        if (!reader->readTextFormat(&value.styles[i])) return false;
        value.styleMap[value.styles[i].first] = value.styles[i].second;
    }

    reader->getTag();
    if (!reader->readSubBlock(3)) return false; // last section

    logDebug("Read pos");
    if (!reader->readDouble(&value.posX)) return false;
    if (!reader->readDouble(&value.posY)) return false;
    // Legacy width
    if (!reader->readFloat(4, &value.width.value)) return false;

    // Modern width? I guess, it just has a timestamp now :)
    if (reader->hasBytesRemaining()) {
        reader->getTag();
        if (!reader->readSubBlock(5)) return false;

        reader->readLwwFloat(1, &value.width);
    }
    return true;
}

json RootTextBlock::toJson() const {
    // TODO: Implement this function
    return {};
}

bool SceneGlyphItemBlock::readValue(TaggedBlockReader *reader) {
    item.value = std::make_optional<GlyphRange>();
    return item.value.value().read(reader);
}

json SceneGlyphItemBlock::toJson() const {
    // TODO: Implement this function
    return {};
}

bool ImageInfoBlock::read(TaggedBlockReader *reader) {
    reader->getTag();
    reader->readSubBlock(1);
    uint64_t subBlocks;
    reader->readValuint(subBlocks);

    for (uint64_t i = 0; i < subBlocks; i++) {
        // Read UUID
        reader->getTag();
        if (!reader->readSubBlock(0)) return false;

        std::string uuid;
        if (!reader->readUUID(uuid, 16)) return false;

        LwwItem<std::string> fileName;
        if (!reader->readLwwString(1, &fileName)) return false;

        LwwItem<std::vector<uint8_t> > unknownFlag;
        if (!reader->readLwwBytes(2, &unknownFlag)) return false;

        images.push_back(ImageInfo{uuid, fileName, unknownFlag});
    }

    return true;
}

json ImageInfoBlock::toJson() const {
    json j;
    for (const auto &image: images) {
        json infoJson;
        infoJson["fileName"] = image.fileName.toJson();
        infoJson["flags"] = image.flags.toJson();
        j[image.uuid] = infoJson;
    }
    return j;
}

bool SceneImageItemBlock::readValue(TaggedBlockReader *reader) {
    ImageItem value;

    LwwItem<std::string> _imageRef;
    if (!reader->readLwwUUID(1, &_imageRef)) return false;
    value.imageRef = _imageRef;

    CrdtId _boundsTimestamp;
    if (!reader->readId(2, &_boundsTimestamp)) return false;
    value.boundsTimestamp = _boundsTimestamp;

    std::vector<float> _vertices;
    uint64_t verticesCount;
    reader->getTag();
    if (!reader->readSubBlock(3)) return false;
    if (!reader->readValuint(verticesCount)) return false;
    for (uint64_t i = 0; i < verticesCount; i++) {
        float vertex;
        reader->readFloat(&vertex);
        _vertices.push_back(vertex);
    }
    value.vertices = _vertices;

    std::vector<uint32_t> _indices;
    uint64_t indicesCount;
    reader->getTag();
    if (!reader->readSubBlock(4)) return false;
    if (!reader->readValuint(indicesCount)) return false;
    for (uint64_t i = 0; i < indicesCount; i++) {
        uint32_t index;
        reader->readInt(&index);
        _indices.push_back(index);
    }
    value.indices = _indices;
    item.value = value;


    return true;
}

json SceneImageItemBlock::toJson() const {
    // TODO: Implement this function
    return {};
}
