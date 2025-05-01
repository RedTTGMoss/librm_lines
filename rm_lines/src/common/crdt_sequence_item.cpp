#include "common/crdt_sequence_item.h"
#include <scene_tree/scene_tree.h>

template<typename T>
void CrdtSequenceItem<T>::applyTreeValue(SceneTree &tree, const CrdtId &nodeId) {
    const auto node = tree.getNode(nodeId);
    _treeValue = *node;
}

template<>
json TextItem::convertValue() const {
    if (std::holds_alternative<std::string>(value.value())) {
        return std::get<std::string>(value.value());
    }
    if (std::holds_alternative<uint32_t>(value.value())) {
        return std::get<uint32_t>(value.value());
    }
    return nullptr;
}

template struct CrdtSequenceItem<Group>;
template struct CrdtSequenceItem<CrdtId>;
template struct CrdtSequenceItem<GlyphRange>;
template struct CrdtSequenceItem<Line>;
template struct CrdtSequenceItem<Text>;
