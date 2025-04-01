// ReSharper disable CppDFAUnusedValue
#include <cstring>
#include <format>
#include <library.h>
#include <common/blocks.h>
#include <reader/tagged_block_reader.h>
#include <common/scene_items.h>

void Block::lookup(Block *&block, const BlockInfo &info) {
    switch (info.blockType) {
        case 0:
            block = new MigrationInfoBlock();
            break;
        case 1:
            block = new SceneTreeBlock();
            break;
        case 2:
            block = new TreeNodeBlock();
            break;
        // case 3:  // TODO
        //     block = new SceneGlyphItemBlock();
        //     break;
        case 4:
            block = new SceneGroupItemBlock();
            break;
        case 5:
            block = new SceneLineItemBlock();
            break;
        // case 6:  // TODO
        //     block = new SceneTextItemBlock();
        //     break;
        case 7:
            block = new RootTextBlock();
            break;
        // case 8: // TODO
        //     block = new SceneTombstoneItemBlock();
        //     break;
        case 9:
            block = new AuthorIdsBlock();
            break;
        case 10:
            block = new PageInfoBlock();
            break;
        case 13:
            block = new SceneInfoBlock();
            break;
        default:
            block = new UnreadableBlock();
            break;
    }
}

Block::Block() {
}

Block::~Block() {
}


bool Block::read(TaggedBlockReader *reader) {
    return false;
}

bool AuthorIdsBlock::read(TaggedBlockReader *reader) {
    uint64_t subBlocks = 0;
    if (!reader->readValuint(subBlocks)) return false;

    for (uint64_t i = 0; i < subBlocks; i++) {
        // Read into subblock
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

bool MigrationInfoBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &migrationId)) return false;
    if (!reader->readBool(2, &isDevice)) return false;
    if (reader->hasBytesRemaining()) {
        // Read unknown
        if (!reader->readBool(3, nullptr)) return false;
    }
    return true;
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

    return true;
}

bool SceneTreeBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &sceneId)) return false;
    if (!reader->readId(2, &nodeId)) return false;
    if (!reader->readBool(3, &isUpdate)) return false;

    if (!reader->readSubBlock(4)) return false;
    if (!reader->readId(1, &parentId)) return false;

    return true;
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

bool SceneItemBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &parentId)) return false;
    if (!reader->readId(2, &item.itemId)) return false;
    if (!reader->readId(3, &item.leftId)) return false;
    if (!reader->readId(4, &item.rightId)) return false;
    if (!reader->readInt(5, &item.deletedLength)) return false;
    bool hasSubBlock;
    if (!reader->checkSubBlock(6, &hasSubBlock)) return false;
    if (hasSubBlock) {
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

bool SceneLineItemBlock::readValue(TaggedBlockReader *reader) {
    version = reader->currentBlockInfo.currentVersion;

    item.value = std::make_optional<Line>();
    return item.value.value().read(reader, version);
}

bool RootTextBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &blockId)) return false;
    if (blockId != CrdtId(0, 0)) return false;

    if (!reader->readSubBlock(2)) return false; // Section one
    for (int i = 1; i <= 2; i++) if (!reader->readSubBlock(1)) return false; // Text items

    uint64_t numberOfItems;
    if (!reader->readValuint(numberOfItems)) return false;

    TextItem textItem;
    for (uint64_t i = 0; i < numberOfItems; i++) {
        textItem = TextItem(); // Reset textItem
        if (!reader->readTextItem(&textItem)) return false;
        value.items.add(textItem);
    }

    for (int i = 2; i >= 1; i--) if (!reader->readSubBlock(i)) return false; // Formatting

    if (!reader->readValuint(numberOfItems)) return false;

    value.styles = std::vector<TextFormat>(numberOfItems);

    for (uint64_t i = 0; i < numberOfItems; i++)
        if (!reader->readTextFormat(&value.styles[i])) return false;

    if (!reader->readSubBlock(3)) return false; // last section

    if (!reader->readDouble(&value.posX)) return false;
    if (!reader->readDouble(&value.posY)) return false;

    if (!reader->readFloat(4, &value.width)) return false;

    return true;
}
