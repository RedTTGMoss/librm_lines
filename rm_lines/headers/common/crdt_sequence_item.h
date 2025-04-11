#ifndef CRDT_SEQUENCE_ITEM_H
#define CRDT_SEQUENCE_ITEM_H

#include <common/data_types.h>

class SceneTree;

template<typename T = std::monostate>
struct CrdtSequenceItem {
public:
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
        return nullptr;
    }

private:
    std::optional<std::reference_wrapper<Group> > _treeValue;
};

typedef CrdtSequenceItem<std::variant<std::string, uint32_t>> TextItem;

template<>
json TextItem::convertValue() const;
template<>
json CrdtSequenceItem<Group>::convertValue() const;

std::string formatTextItem(TextItem textItem);

#endif //CRDT_SEQUENCE_ITEM_H
