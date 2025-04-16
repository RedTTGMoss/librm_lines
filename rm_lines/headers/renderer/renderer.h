#ifndef RENDERER_H
#define RENDERER_H

#include <common/scene_tree.h>
#include <unordered_map>

#define TEXT_TOP_Y -88

constexpr std::array<std::pair<ParagraphStyle, int>, 8> lineHeights = {{
    {BASIC, 100},
    {PLAIN, 71},
    {HEADING, 150},
    {BOLD, 70},
    {BULLET, 35},
    {BULLET2, 35},
    {CHECKBOX, 100},
    {CHECKBOX_CHECKED, 100}
}};

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
