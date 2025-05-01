#pragma once

#include <common/data_types.h>

class SceneTree;

template<typename T = std::monostate>
struct CrdtSequenceItem {
    CrdtId itemId;
    CrdtId leftId;
    CrdtId rightId;
    uint32_t deletedLength;
    std::optional<T> value = std::nullopt;

    void applyTreeValue(SceneTree &tree, const CrdtId &nodeId);

    [[nodiscard]] std::optional<std::reference_wrapper<Group> > getTreeValue() const {
        return _treeValue;
    }

    json toJson() const {
        json j = toJsonNoItem();
        j["itemId"] = itemId.toJson();
        return j;
    }

    json toJsonNoItem() const {
        return {
            {"leftId", leftId.toJson()},
            {"rightId", rightId.toJson()},
            {"deletedLength", deletedLength},
            {"value", value.has_value() ? convertValue() : nullptr}
        };
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    json convertValue() const {
        return value.value().toJson();
    }

    CrdtSequenceItem() = default;

    explicit CrdtSequenceItem(
        const CrdtId itemId,
        const CrdtId leftId = END_MARKER,
        const CrdtId rightId = END_MARKER,
        const uint32_t deletedLength = 0,
        std::optional<T> value = std::nullopt) : itemId(itemId),
                                                 leftId(leftId),
                                                 rightId(rightId),
                                                 deletedLength(deletedLength),
                                                 value(value) {
    };

private:
    std::optional<std::reference_wrapper<Group> > _treeValue;
};

typedef CrdtSequenceItem<std::variant<std::string, uint32_t> > TextItem;

template<>
json TextItem::convertValue() const;

std::string formatTextItem(TextItem textItem);

std::string reprTextItem(TextItem textItem);
