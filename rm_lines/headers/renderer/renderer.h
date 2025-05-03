#pragma once

#include <scene_tree/scene_tree.h>
#include <unordered_map>

#include "advanced/document_size_tracker.h"
#include "advanced/text.h"
#include "advanced/layers.h"

#define TEXT_TOP_Y (-88)
static constexpr CrdtId TEXT_LAYER{7, 1};

class Renderer {
public:
    TextDocument textDocument = TextDocument();
    std::unordered_map<CrdtId, uint32_t> anchors;
    std::vector<Layer> layers;

    explicit Renderer(SceneTree *sceneTree, PageType pageType, bool landscape);

    ~Renderer();

    void prepareTextDocument();

    DocumentSizeTracker *getSizeTracker(CrdtId layerId);

    DocumentSizeTracker *initSizeTracker(CrdtId layerId);

    void trackX(const CrdtId &layerId, float posX);

    void trackY(const CrdtId &layerId, float posY);

    void calculateAnchors();

    void groupLines(Layer &layer, CrdtId parentId, CrdtId groupId, int offsetX = 0, int offsetY = 0);

    json getParagraphs() const;

    json getLayers() const;

    // Exports
    void toMd(std::ostream &stream) const;

    void toTxt(std::ostream &stream) const;

    void toHtml(std::ostream &stream);

private:
    SceneTree *sceneTree;
    IntPair paperSize;
    bool landscape;
    PageType pageType;
    std::unordered_map<CrdtId, DocumentSizeTracker> sizeTrackers;
};
