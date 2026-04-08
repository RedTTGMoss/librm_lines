#include "scene_tree/scene_tree_editor.h"

CrdtId SceneTreeEditor::createLayer(const std::string &label) {
    const auto layerId = _idCounter;
    const auto node = createSceneTree(layerId, label);
    addSceneTree(std::move(node));
    _layers.push_back(layerId);
    return layerId;
}

Group SceneTreeEditor::createSceneTree(const CrdtId &id, const std::string &label) {
    Group group;
    group.parentId = ROOT_NODE;
    group.updated = true; // This ensures it will be written
    _idCounter = id;
    group.nodeId = id;
    if (!label.empty()) {
        _idCounter++;
        group.label.timestamp = id + 1;
        group.label.value = label;
    }
    return group;
}

void SceneTreeEditor::addSceneTree(const Group &&node) {
    _nodeIds[node.nodeId] = std::make_unique<Group>(std::move(node));
    _nodeIds[node.nodeId].get()->parentIs = TREE;

    CrdtSequenceItem<CrdtId> childItem;
    childItem.itemId = _idCounter++;
    childItem.value = node.nodeId;
    addItem(childItem, node.parentId);
}

void SceneTreeEditor::init() {
    // Initialize authorIDs
    authorsInfo = AuthorIdsBlock();

    // Initialize migration info
    migrationInfo = MigrationInfoBlock();

    // Initialize Layer 1
    createLayer("Layer 1");
    _idCounter.first++;

    // Initialize SceneInfo
    sceneInfo = SceneInfoBlock();
}
