#pragma once

#include <unordered_map>
#include <optional>
#include <string>
#include <advanced/math.h>
#include <variant>
#include <format>
#include <library.h>
#include <unordered_set>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct CrdtId {
    uint8_t first;
    uint64_t second;

    bool operator==(const CrdtId &crdt_id) const = default;

    bool operator<(const CrdtId &other) const {
        return std::tie(first, second) < std::tie(other.first, other.second);
    }

    [[nodiscard]] std::string repr() const;

    [[nodiscard]] json toJson() const;

    CrdtId operator++(int);
};

template<>
struct std::hash<CrdtId> {
    size_t operator()(const CrdtId &crdtId) const noexcept {
        // Combine the hashes of 'first' and 'second' using std::hash
        return std::hash<uint8_t>{}(crdtId.first) ^ (std::hash<uint64_t>{}(crdtId.second) << 1);
    }
};

enum Side {
    LEFT,
    RIGHT
};


static constexpr CrdtId END_MARKER{0, 0};


template<typename T>
struct CrdtSequence {
    std::unordered_map<CrdtId, T> sequence;

    void add(T &&item) {
        sequence[item.itemId] = std::move(item);
    }

    [[nodiscard]] std::vector<CrdtId> getSortedIds() const {
        // Skip all the processing ahead, if the sequence is empty
        if (sequence.empty())
            return {};

        // Allocate size for the final sorted keys
        std::vector<CrdtId> sortedIds;
        sortedIds.reserve(sequence.size());

        // We initialize a list containing the IDs and their dependant IDs
        std::unordered_map<CrdtId, std::unordered_set<CrdtId> > graph;

        // Lambda to get the respective sideId depending on the side passed
        auto getSideId = [&](const T item, const Side side) -> std::optional<CrdtId> {
            const auto sideId = side == LEFT ? item.leftId : item.rightId;
            if (sideId == END_MARKER || !sequence.contains(sideId))
                return std::nullopt;
            return sideId;
        };

        for (const auto &[itemId, item]: sequence) {
            // Get the side IDs of this item
            auto leftSideId = getSideId(item, LEFT);
            auto rightSideId = getSideId(item, RIGHT);

            // Graph out which item needs which sideId
            if (leftSideId.has_value() && sequence.contains(leftSideId.value()))
                graph[itemId].insert(leftSideId.value()); // This item first needs the left side
            else // If the sideId is not present in the sequence of items
                graph[itemId] = {}; // Make an empty set for the item on the graph

            if (rightSideId.has_value() && sequence.contains(rightSideId.value()))
                // If the right side is present in the sequence
                graph[rightSideId.value()].insert(itemId); // This item first needs the right side
        }

        // Debug the current graph

        std::vector<CrdtId> nextIds;
        while (!graph.empty()) {
            for (auto it = graph.begin(); it != graph.end();) {
                // If the item has no dependencies, we can add it to the next items
                if (it->second.empty()) {
                    nextIds.push_back(it->first);
                    it = graph.erase(it); // Remove the item from the graph
                } else {
                    ++it;
                }
            }

            if (nextIds.empty() && !graph.empty()) {
                // If we have no next items but the graph is not finished, we have a cycle
                std::string debugGraph = "Here's the dependency graph, while sorting the sequence: ";
                for (const auto &[key, value]: graph) {
                    debugGraph += std::format("\nItem: {} -> ", key.repr());
                    for (const auto &dep: value) {
                        debugGraph += std::format("{} ", dep.repr());
                    }
                }
                logError(debugGraph);
                throw std::runtime_error("Cyclic dependency in sequence");
            }

            // Sort descending
            std::sort(nextIds.begin(), nextIds.end(), [](const CrdtId &a, const CrdtId &b) {
                return a < b;
            });

            for (const auto &itemId: nextIds) {
                // Add the item to the sorted keys
                sortedIds.push_back(itemId);

                // Remove the item as a dependency of all other items in the graph
                for (auto &dependencies: graph | std::views::values)
                    dependencies.erase(itemId);
            }
            // Erase the list of items for the next loop cycle
            nextIds.clear();
        }

        return sortedIds;
    }

    T &operator[](const CrdtId &key) {
        return sequence[key];
    }

    [[nodiscard]] json toJson() const {
        json j;
        // Iterate over the map and convert each item to JSON
        for (const auto &[key, value]: sequence) {
            j[key.toJson()] = value.toJsonNoItem();
        }
        return j;
    }

    // Allow for itteration
    auto begin() {
        return sequence.begin();
    }

    auto end() {
        return sequence.end();
    }

    // Add const versions for const CrdtSequence
    auto begin() const {
        return sequence.begin();
    }

    auto end() const {
        return sequence.end();
    }
};

struct IntPair {
    uint32_t first;
    uint32_t second;
};

template<typename T>
struct LwwItem {
    CrdtId timestamp = END_MARKER;
    T value;

    explicit LwwItem(T value) : value(value) {
    };

    explicit LwwItem(const CrdtId timestamp, T value) : timestamp(timestamp), value(value) {
    };

    LwwItem() = default;

    [[nodiscard]] json toJson() const {
        return {
            {"timestamp", timestamp.toJson()},
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
    // BGRA (the bite order is important, it's how it's stored in the file)
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;

    json toJson() const;

    uint32_t toRGBA() const;
};
