#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <format>
#include <library.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct CrdtId {
    uint8_t first;
    uint64_t second;

    bool operator==(const CrdtId &crdt_id) const = default;

    bool operator<(const CrdtId &other) const {
        return std::tie(first, second) < std::tie(other.first, other.second);
    }

    std::string repr() const;

    json toJson() const;
};

template<>
struct std::hash<CrdtId> {
    size_t operator()(const CrdtId &crdtId) const noexcept {
        // Combine the hashes of 'first' and 'second' using std::hash
        return std::hash<uint8_t>{}(crdtId.first) ^ (std::hash<uint64_t>{}(crdtId.second) << 1);
    }
};

template<typename T>
struct CrdtSequence {
    std::map<CrdtId, T> sequence;

    void add(T &item) {
        sequence[item.itemId] = item;
    }

    json toJson() {
        json j;
        // Iterate over the map and convert each item to JSON
        for (const auto &[key, value] : sequence) {
            j[key.toJson()] = value.toJsonNoItem();
        }
        return j;
    }
};

struct IntPair {
    uint32_t first;
    uint32_t second;
};

struct Rect {
    double x;
    double y;
    double w;
    double h;
};

template<typename T>
struct LwwItem {
    CrdtId itemId;
    T value;

    json toJson() const {
        return {
            {"itemId", itemId.toJson()},
            {"value", value}
        };
    }
};


class Group {
public:
    explicit Group(CrdtId nodeId);

    Group() = default;

    CrdtId nodeId;
    LwwItem<std::string> label;
    LwwItem<bool> visible;
    std::optional<LwwItem<CrdtId> > anchorId;
    std::optional<LwwItem<uint8_t> > anchorType;
    std::optional<LwwItem<float> > anchorThreshold;
    std::optional<LwwItem<float> > anchorOriginX;

    json toJson() const;
    json toJsonNoItem() const;
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

template<>
json LwwItem<ParagraphStyle>::toJson() const;
template<>
json LwwItem<CrdtId>::toJson() const;

typedef std::pair<std::string, std::optional<uint32_t> > StringWithFormat;
typedef std::pair<CrdtId, LwwItem<ParagraphStyle> > TextFormat;

json textFormatToJson(const TextFormat &textFormat);

struct Color {
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

#endif //DATA_TYPES_H
