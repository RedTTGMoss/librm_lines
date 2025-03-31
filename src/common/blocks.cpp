// ReSharper disable CppDFAUnusedValue
#include <cstring>
#include <format>
#include <library.h>
#include <common/blocks.h>
#include <reader/tagged_block_reader.h>
#include <uuid/uuid.h>

void Block::lookup(Block *&block, const BlockInfo &info) {
    switch (info.block_type) {
        case 9:
            block = new AuthorIdsBlock();
            break;
        default:
            block = new UnreadableBlock();
    }
}

Block::Block() {
}

Block::~Block() {
}


bool Block::read(TaggedBlockReader *reader, BlockInfo &info) {
    return false;
}

bool AuthorIdsBlock::read(TaggedBlockReader *reader, BlockInfo &info) {
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

        author_ids[authorId] = uuid;
    }

    return true;
}
