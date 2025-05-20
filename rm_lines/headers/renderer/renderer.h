#pragma once


#include "renderer/rm_lines_stroker/rm_lines_stroker.h"
#include "renderer/rm_lines_stroker/rm_pens/rm_pen_fill.h"
#include "advanced/document_size_tracker.h"
#include <scene_tree/scene_tree.h>
#include "advanced/layers.h"
#include "advanced/text.h"
#include <unordered_map>
#define TEXT_TOP_Y (-88)

using ImageBuffer = RMLinesRenderer::ImageBuffer;
using VaryingGeneratorLengthWidth = RMLinesRenderer::VaryingGeneratorLengthWidth;
using CapStyle = RMLinesRenderer::CapStyle;
using JoinStyle = RMLinesRenderer::JoinStyle;
using Varying2D = RMLinesRenderer::Varying2D;
using Varying4D = RMLinesRenderer::Varying4D;


static constexpr CrdtId TEXT_LAYER{7, 1};

class Renderer {
public:
    TextDocument textDocument = TextDocument();
    std::unordered_map<CrdtId, uint32_t> anchors;
    std::vector<Layer> layers;
    IntPair paperSize;
    bool landscape;
    PageType pageType;

    explicit Renderer(SceneTree *sceneTree, PageType pageType, bool landscape);

    ~Renderer() = default;

    void prepareTextDocument();

    DocumentSizeTracker *getSizeTracker(CrdtId layerId);

    DocumentSizeTracker *initSizeTracker(CrdtId layerId);

    [[nodiscard]] auto trackX(const CrdtId &layerId, float posX);

    [[nodiscard]] auto trackY(const CrdtId &layerId, float posY);

    void calculateAnchors();

    void groupLines(Layer &layer, CrdtId parentId, CrdtId groupId, int offsetX = 0, int offsetY = 0);

    json getParagraphs() const;

    json getLayers() const;

    // Exports
    void toMd(std::ostream &stream) const;

    void toTxt(std::ostream &stream) const;

    void toHtml(std::ostream &stream);

    void getFrame(uint32_t *data, size_t dataSize, Vector position, Vector size, float scale, bool antialias);

private:
    SceneTree *sceneTree;
    std::unordered_map<CrdtId, DocumentSizeTracker> sizeTrackers;
    RMLinesRenderer::Stroker<RMLinesRenderer::ClippedRaster<RMLinesRenderer::LerpRaster<rMPenFill> >,
        VaryingGeneratorLengthWidth> stroker;
};
