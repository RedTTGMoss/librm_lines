#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstdint>

struct BlockInfo {
    uint8_t offset;
    uint32_t size;
    uint8_t min_version;
    uint8_t current_version;
    uint8_t block_type;
};

#endif //BLOCKS_H
