#ifndef RENDERER_H
#define RENDERER_H

#include <common/scene_tree.h>
#include <unordered_map>

#define TEXT_TOP_Y -88

class Renderer {
public:
    explicit Renderer(SceneTree *sceneTree);

    ~Renderer();

    void calculateAnchors();

private:
    SceneTree *sceneTree;
    std::unordered_map<CrdtId, float> anchors;
};

#endif //RENDERER_H
