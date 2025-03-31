// ReSharper disable CppDFAUnusedValue
#include <cstring>
#include <format>
#include <library.h>
#include <common/blocks.h>
#include <reader/tagged_block_reader.h>

void Block::lookup(Block *&block, const BlockInfo &info) {
    switch (info.block_type) {
        case 0:
            block = new MigrationInfoBlock();
            break;
        case 9:
            block = new AuthorIdsBlock();
            break;
        case 10:
            block = new PageInfoBlock();
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

bool MigrationInfoBlock::read(TaggedBlockReader *reader, BlockInfo &info) {
    if (!reader->readId(1, &migrationId)) return false;
    if (!reader->readBool(2, &isDevice)) return false;
    if (reader->bytesRemainingInBlock()) {
        // Read unknown
        if (!reader->readBool(3, nullptr)) return false;
    }
    return true;
}

bool PageInfoBlock::read(TaggedBlockReader *reader, BlockInfo &info) {
    if (!reader->readInt(1, &loads_count)) return false;
    if (!reader->readInt(2, &merges_count)) return false;
    if (!reader->readInt(3, &text_chars_count)) return false;
    if (!reader->readInt(4, &text_lines_count)) return false;
    if (reader->bytesRemainingInBlock()) {
        // Read type_folio_use_count
        if (!reader->readInt(5, &type_folio_use_count)) return false;
    }
    return true;
}
