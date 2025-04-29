#ifndef RENDERER_H
#define RENDERER_H

#include <scene_tree/scene_tree.h>
#include <unordered_map>

#include "advanced/document_size_tracker.h"
#include "advanced/text.h"

#define TEXT_TOP_Y (-88)

class Renderer {
public:
    explicit Renderer(SceneTree *sceneTree, PageType pageType, bool landscape);

    ~Renderer();

    void prepareTextDocument();

    void trackX(float posX);

    void trackY(float posY);

    void calculateAnchors();

    json getParagraphs() const;

    void toMd(std::ostream& stream) const;

    void toHtml(std::ostream& stream);

private:
    SceneTree *sceneTree;
    DocumentSizeTracker _sizeTracker;
    TextDocument textDocument = TextDocument();
    std::unordered_map<CrdtId, uint32_t> anchors;
};

#endif //RENDERER_H
