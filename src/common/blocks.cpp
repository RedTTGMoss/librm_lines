// ReSharper disable CppDFAUnusedValue
#include <cstring>
#include <format>
#include <library.h>
#include <common/blocks.h>
#include <reader/tagged_block_reader.h>

void Block::lookup(Block *&block, const BlockInfo &info) {
    switch (info.blockType) {
        case 0:
            block = new MigrationInfoBlock();
            break;
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
    
}
