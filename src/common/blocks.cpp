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
        // case 7:  // TODO
        //     block = new RootTextBlock();
        //     break;
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

    const auto subBlockInfo = new SubBlockInfo();

    for (uint64_t i = 0; i < subBlocks; i++) {
        if (!reader->readSubBlock(0, *subBlockInfo)) return false;
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
    if (reader->bytesRemainingInBlock()) {
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
    if (reader->bytesRemainingInBlock()) {
        // Read typeFolioUseCount
        if (!reader->readInt(5, &typeFolioUseCount)) return false;
    }
    return true;
}

bool SceneInfoBlock::read(TaggedBlockReader *reader) {
    if (!reader->readLwwId(1, &currentLayer)) return false;
    if (reader->bytesRemainingInBlock()) {
        LwwItem<bool> _backgroundVisible;
        if (!reader->readLwwBool(2, &_backgroundVisible)) return false;
        backgroundVisible = _backgroundVisible;
    }
    if (reader->bytesRemainingInBlock()) {
        LwwItem<bool> _rootDocumentVisible;
        if (!reader->readLwwBool(3, &_rootDocumentVisible)) return false;
        rootDocumentVisible = _rootDocumentVisible;
    }
    if (reader->bytesRemainingInBlock()) {
        LwwItem<IntPair> _paperSize;
        if (!reader->readLwwIntPair(5, &_paperSize)) return false;
        paperSize = _paperSize;
    }

    return true;
}

bool SceneTreeBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &sceneId)) return false;
    if (!reader->readId(2, &nodeId)) return false;
    if (!reader->readBool(3, &isUpdate)) return false;
    if (SubBlockInfo subBlockInfo; !reader->readSubBlock(4, subBlockInfo)) return false;
    if (!reader->readId(1, &parentId)) return false;

    return true;
}

bool TreeNodeBlock::read(TaggedBlockReader *reader) {
    if (!reader->readId(1, &group.nodeId)) return false;
    if (!reader->readLwwString(2, &group.label)) return false;
    if (!reader->readLwwBool(3, &group.visible)) return false;
    if (reader->bytesRemainingInBlock()) {
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
    if (!reader->readId(2, &itemId)) return false;
    if (!reader->readId(3, &leftId)) return false;
    if (!reader->readId(4, &rightId)) return false;
    if (!reader->readInt(5, &deletedLength)) return false;
    bool hasSubBlock;
    if (!reader->checkSubBlock(6, &hasSubBlock)) return false;
    if (hasSubBlock) {
        if (SubBlockInfo subBlockInfo; !reader->readSubBlock(6, subBlockInfo)) return false;
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
    value = _value;
    return true;
}

bool SceneLineItemBlock::readValue(TaggedBlockReader *reader) {
    version = reader->currentBlockInfo.currentVersion;

    value = std::make_optional<Line>();
    return value.value().read(reader, version);
}


