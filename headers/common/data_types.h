#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>

struct CrdtId {
    uint8_t first;
    uint64_t second;
};

struct IntPair {
    uint64_t first;
    uint64_t second;
};

template <typename T>
struct LwwItem {
    CrdtId timestamp;
    T value;
};

#endif //DATA_TYPES_H
