#include "scene_tree/scene_tree.h"

SceneTree::SceneTree() {
    // Add the root node
    addNode(ROOT_NODE, BLANK_NODE, BLANK_NODE);
}

void SceneTree::addNode(const CrdtId &nodeId, const CrdtId &parentNodeId, const CrdtId &parentTreeId) {
    if (_nodeIds.contains(nodeId)) {
        throw std::invalid_argument(std::format("Node {} already exists in the tree", nodeId.repr()));
    }
    if (nodeId != ROOT_NODE && parentNodeId != BLANK_NODE && parentTreeId != BLANK_NODE) {
        throw std::runtime_error("A node with a node parent and a tree parent, is not supported and is invalid data");
    }
    if (parentNodeId != BLANK_NODE) {
        logDebug(std::format("Add node {}, with node parent {}", nodeId.repr(), parentNodeId.repr()));
        _nodeIds[nodeId] = std::make_unique<Group>(nodeId, parentNodeId);
        _nodeIds[nodeId]->parentIsNode = true;
    } else {
        logDebug(std::format("Add node {}, with tree parent {}", nodeId.repr(), parentTreeId.repr()));
        _nodeIds[nodeId] = std::make_unique<Group>(nodeId, parentTreeId);
    }
}

void SceneTree::addItem(const SceneItemVariant &item, const CrdtId &parentId) {
    _groupChildren[parentId].push_back(item);
}

Group *SceneTree::getNode(const CrdtId &nodeId) {
    // logDebug(std::format("Get node {}", id.repr()));
    const auto it = _nodeIds.find(nodeId);

    if (it == _nodeIds.end()) {
        throw std::out_of_range("Node not found in the tree");
    }
    if (it->second == nullptr) {
        throw std::runtime_error("Node is null");
    }

    return it->second.get();
}

std::vector<SceneItemVariant> SceneTree::getGroupChildren(const CrdtId &nodeId) {
    return _groupChildren[nodeId];
}

json SceneTree::toJson() {
    json j = {
        {"authorsInfo", authorsInfo ? authorsInfo->toJson() : nullptr},
        {"migrationInfo", migrationInfo ? migrationInfo->toJson() : nullptr},
        {"pageInfo", pageInfo ? pageInfo->toJson() : nullptr},
        {"sceneInfo", sceneInfo ? sceneInfo->toJson() : nullptr},
        {"imageInfo", imageInfo ? imageInfo->toJson() : nullptr},
        {"rootText", rootText ? rootText->toJson() : nullptr},
        {"nodes", json()}
    };

    for (const auto &[key, value]: _nodeIds) {
        json nodeJson = value->toJsonNoItem();
        nodeJson["children"] = json();

        for (const auto &child: _groupChildren[key]) {
            if (auto item = std::get_if<CrdtSequenceItem<Group> >(&child)) {
                json obj = item->toJson();
                obj["_type"] = "Group";
                nodeJson["children"].push_back(obj);
            } else if (auto item = std::get_if<CrdtSequenceItem<CrdtId> >(&child)) {
                json obj = item->toJson();
                obj["_type"] = "CrdtId";
                nodeJson["children"].push_back(obj);
            } else if (auto item = std::get_if<CrdtSequenceItem<GlyphRange> >(&child)) {
                json obj = item->toJson();
                obj["_type"] = "GlyphRange";
                nodeJson["children"].push_back(obj);
            } else if (auto item = std::get_if<CrdtSequenceItem<Line> >(&child)) {
                json obj = item->toJson();
                obj["_type"] = "Line";
                nodeJson["children"].push_back(obj);
            } else if (auto item = std::get_if<CrdtSequenceItem<Text> >(&child)) {
                json obj = item->toJson();
                obj["_type"] = "Text";
                nodeJson["children"].push_back(obj);
            } else if (auto item = std::get_if<CrdtSequenceItem<ImageItem> >(&child)) {
                json obj = item->toJson();
                obj["_type"] = "Image";
                nodeJson["children"].push_back(obj);
            } else {
                nodeJson["children"].push_back(nullptr);
            }
        }

        j["nodes"][key.toJson()] = nodeJson;
    }

    return j;
}
