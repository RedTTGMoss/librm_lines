// ReSharper disable CppDFAUnusedValue
#include <common/blocks.h>

void Block::lookup(const Block *block, const BlockInfo &info) {
    switch (info.block_type) {
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
