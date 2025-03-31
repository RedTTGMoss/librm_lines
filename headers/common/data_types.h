#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>
#include <optional>

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

struct Group {
    CrdtId nodeId;
    LwwItem<std::string> label;
    LwwItem<bool> visible;
    std::optional<LwwItem<CrdtId>> anchorId;
    std::optional<LwwItem<uint8_t>> anchorType;
    std::optional<LwwItem<float>> anchorThreshold;
    std::optional<LwwItem<float>> anchorOriginX;
};

#endif //DATA_TYPES_H
