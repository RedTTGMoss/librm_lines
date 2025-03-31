#include <reader/tagged_block_reader.h>
#include <cstring>
#include <format>
#include <library.h>

bool TaggedBlockReader::readBlockInfo(BlockInfo &block_info) {
    if (current_offset + 8 > data_size_) return false;
    // Ensure enough data (4 + 1 + 1 + 1)

    block_info.offset = current_offset + 8; // Store offset at block data start
    std::memcpy(&block_info.size, data_ + current_offset, sizeof(uint32_t));
    current_offset += 4; // Move past block_size

    current_offset += 1; // Skip 'unknown'

    block_info.min_version = data_[current_offset];
    block_info.current_version = data_[current_offset + 1];
    block_info.block_type = data_[current_offset + 2];

    current_offset += 3; // Move forward by 3 bytes

    return true;
}

bool TaggedBlockReader::readBlock(Block *block, BlockInfo &block_info) {
    Block::lookup(block, block_info);
    return block ? block->read(this, block_info) : false;
}

bool TaggedBlockReader::buildTree() {
    return false;
}

