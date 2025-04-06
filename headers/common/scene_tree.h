#ifndef SCENE_TREE_H
#define SCENE_TREE_H
#include <unordered_map>


#include <memory>
#include <common/data_types.h>

class SceneTree {
public:
    SceneTree ();

    void addNode(const CrdtId &nodeId, const CrdtId &parentId);
    // bool addItem(const CrdtSequenceItem<SceneItem> &nodeId, const CrdtId &parentId);
    Group* getNode(const CrdtId &nodeId);
private:
    std::unordered_map<CrdtId, std::unique_ptr<Group>> _nodeIds;
};

#endif //SCENE_TREE_H
