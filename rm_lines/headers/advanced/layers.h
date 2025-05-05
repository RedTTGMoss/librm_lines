#pragma once
#include "common/data_types.h"
#include "scene_tree/scene_tree.h"
#include <nlohmann/json.hpp>

#include "line_info.h"

using json = nlohmann::json;

/*
 *A header only group / layer representation
 *for the node tree structure that reMarkable uses
 *this simplifies the process of rendering
*/

constexpr CrdtId LAYER_INFO_NODE{0, 1};

class Layer {
public:
    Layer(SceneTree *tree, const CrdtId groupId) : groupId(groupId), tree(tree) {
        const auto group = tree->getNode(groupId);
        assert(group != nullptr);
        visible = group->visible.value;
    }

    ~Layer() = default;

    std::string getLabel() const {
        return tree->getNode(groupId)->label.value;
    }

    json toJson() const {
        return {
            {"groupId", groupId.toJson()},
            {"visible", visible},
            {"label", getLabel()}
        };
    }

    CrdtId groupId;
    bool visible;
    std::vector<LineInfo> lines;
private:
    SceneTree *tree;
};

inline std::vector<Layer> layersFromSceneTree(SceneTree *tree) {
    std::vector<Layer> layers;
    for (const auto layerInfo = tree->getGroupChildren(LAYER_INFO_NODE); const auto &child : layerInfo) {
        assert(std::holds_alternative<CrdtSequenceItem<CrdtId> >(child));
        auto item = std::get<CrdtSequenceItem<CrdtId> >(child);
        if(!item.value.has_value())
            continue;
        auto groupId = item.value.value();
        const auto group = tree->getNode(groupId);
        assert(group != nullptr);

        Layer layer(tree, groupId);
        layers.push_back(layer);

    }
    return layers;
}
