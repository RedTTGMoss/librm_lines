#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <format>

struct CrdtId {
    uint8_t first;
    uint64_t second;

    bool operator==(const CrdtId & crdt_id) const = default;
    bool operator<(const CrdtId& other) const {
        return std::tie(first, second) < std::tie(other.first, other.second);
    }
};

template <typename T = std::monostate>
struct CrdtSequenceItem {
    CrdtId itemId;
    CrdtId leftId;
    CrdtId rightId;
    uint32_t deletedLength;
    std::optional<T> value = std::nullopt;
};

template <typename T>
struct CrdtSequence {
    std::map<CrdtId, T> sequence;
    void add(T &item) {
        sequence[item.itemId] = item;
    }
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

enum ParagraphStyle {
    BASIC = 0,
    PLAIN = 1,
    HEADING = 2,
    BOLD = 3,
    BULLET = 4,
    BULLET2 = 5,
    CHECKBOX = 6,
    CHECKBOX_CHECKED = 7
};

typedef CrdtSequenceItem<std::variant<std::string, uint32_t>> TextItem;
typedef std::pair<std::string, std::optional<uint32_t>> StringWithFormat;
typedef std::pair<CrdtId, LwwItem<ParagraphStyle>> TextFormat;

std::string formatTextItem(TextItem textItem);

#endif //DATA_TYPES_H
