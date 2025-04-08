#ifndef SCENE_TREE_H
#define SCENE_TREE_H
#include <unordered_map>


#include <memory>
#include <common/data_types.h>

#include <variant>
#include <common/data_types.h>
#include <common/crdt_sequence_item.h>
#include <common/scene_items.h>

using SceneItemVariant = std::variant<
    CrdtSequenceItem<Group>,
    CrdtSequenceItem<CrdtId>,
    CrdtSequenceItem<GlyphRange>,
    CrdtSequenceItem<Line>,
    CrdtSequenceItem<Text>
>;

class SceneTree {
public:
    SceneTree();

    void addNode(const CrdtId &nodeId, const CrdtId &parentId);

    void addItem(const SceneItemVariant &item, const CrdtId &parentId);

    Group *getNode(const CrdtId &nodeId);

private:
    std::unordered_map<CrdtId, std::unique_ptr<Group> > _nodeIds;
    std::unordered_map<CrdtId, std::vector<SceneItemVariant> > _groupChildren;
};

#endif //SCENE_TREE_H
