#pragma once
#include <stack>

#include "scene_tree.h"

// A class that expands on the scene tree with functions for building out the scene tree
class SceneTreeEditor final : public SceneTree {
public:
    CrdtId createLayer(const std::string &label = "");

    SceneInfoBlock createSceneInfo();

    Group createSceneTree(const CrdtId &id, const std::string &label = "");

    void addSceneTree(const Group &&node);

    void init();

private:
    CrdtId _idCounter = ROOT_TEXT_NODE;
    std::vector<CrdtId> _layers;
};
