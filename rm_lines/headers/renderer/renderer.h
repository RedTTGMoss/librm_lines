#ifndef RENDERER_H
#define RENDERER_H

#include <common/scene_tree.h>
#include <unordered_map>

#include "advanced/text.h"

#define TEXT_TOP_Y (-88)

class Renderer {
public:
    explicit Renderer(SceneTree *sceneTree);

    ~Renderer();

    void prepareTextDocument();

    void calculateAnchors();

private:
    SceneTree *sceneTree;
    TextDocument textDocument = TextDocument();
    std::unordered_map<CrdtId, float> anchors;
};

#endif //RENDERER_H
