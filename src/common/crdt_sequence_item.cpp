#include "common/crdt_sequence_item.h"
#include <common/scene_tree.h>

template<typename T>
void CrdtSequenceItem<T>::applyTreeValue(SceneTree &tree, const CrdtId &nodeId) {
    auto node = tree.getNode(nodeId);
    _treeValue = *node;
}

template struct CrdtSequenceItem<Group>;
template struct CrdtSequenceItem<CrdtId>;
template struct CrdtSequenceItem<GlyphRange>;
template struct CrdtSequenceItem<Line>;
template struct CrdtSequenceItem<Text>;
