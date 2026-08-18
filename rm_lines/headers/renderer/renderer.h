#pragma once


#include "renderer/rm_lines_stroker/rm_lines_stroker.h"
#include "renderer/rm_lines_stroker/rm_pens/rm_pen_fill.h"
#include "advanced/document_size_tracker.h"
#include <scene_tree/scene_tree.h>
#include "advanced/layers.h"
#include "advanced/text.h"
#include "advanced/text_scale.h"
#include <unordered_map>

#include "image_ref.h"
#include "text/text_renderer.h"

class Renderer;
using ImageBuffer = RMLinesRenderer::ImageBuffer;
using VaryingGeneratorLengthWidth = RMLinesRenderer::VaryingGeneratorLengthWidth;
using CapStyle = RMLinesRenderer::CapStyle;
using JoinStyle = RMLinesRenderer::JoinStyle;
using Varying2D = RMLinesRenderer::Varying2D;
using Varying4D = RMLinesRenderer::Varying4D;


static constexpr CrdtId TEXT_LAYER{7, 1};

typedef void TemplateOperationFunction(rMPenFill *fill, Renderer *renderer);

struct RendererConfig {
    uint8_t configVersion = 1;
    int8_t penWhitelist[10] = {};
    int8_t penBlacklist[10] = {};
    bool useWhitelist = false;
    CrdtId disabledLayers[10] = {};
    bool enableText = true;
    bool enableImages = true;
    bool enableGlyphHighlights = true;

    RendererConfig() {
        std::ranges::fill(penWhitelist, -1);
        std::ranges::fill(penBlacklist, -1);
        std::ranges::fill(disabledLayers, END_MARKER);
    }
};

class Renderer {
public:
    TextDocument textDocument = TextDocument();
    std::unordered_map<CrdtId, uint32_t> anchors;
    std::vector<Layer> layers;
    IntPair paperSize;
    RendererConfig config;
    bool landscape;
    PageType pageType;
    TemplateOperationFunction *templateFunction = nullptr;
    TextRenderer *textRenderer;
    std::string templateName = "Blank";

    explicit Renderer(SceneTree *sceneTree, PageType pageType, bool landscape);

    ~Renderer() = default;

    void prepareTextDocument();

    DocumentSizeTracker *getSizeTracker(CrdtId layerId);

    DocumentSizeTracker *initSizeTracker(CrdtId layerId);

    auto trackX(const CrdtId &layerId, float posX);

    auto trackY(const CrdtId &layerId, float posY);

    void calculateAnchors();

    void groupLayerItems(Layer &layer, CrdtId parentId, CrdtId groupId, int offsetX = 0, int offsetY = 0);

    json getParagraphs() const;

    json getAnchors() const;

    json getLayers() const;

    json getLayerFull(CrdtId layerId) const;

    float getTextMargin() const;

    float getTextWidth() const;

    // Exports
    void toMd(std::ostream &stream) const;

    void toRM(std::ostream &stream);

    void toTxt(std::ostream &stream) const;

    void toHtml(std::ostream &stream);

    void getFrame(uint32_t *data, size_t dataSize, Vector position, Vector frameSize, Vector bufferSize,
                  bool antialias);

    void setTemplate(const std::string &templateName);

    RMLinesRenderer::Stroker<RMLinesRenderer::ClippedRaster<RMLinesRenderer::LerpRaster<rMPenFill> >,
        VaryingGeneratorLengthWidth> *getStroker() {
        return &stroker;
    }

    // Images
    void addImage(const char *uuid, const char *path);

    void addImage(const std::string &uuid, const std::string &path) {
        return addImage(uuid.c_str(), path.c_str());
    }

    friend class TaggedBlockWriter;
    friend class TextRenderer;

private:
    SceneTree *sceneTree;
    Vector frameSize;
    std::unordered_map<std::string, std::shared_ptr<ImageRef> > imageRefMap;
    std::unordered_map<CrdtId, DocumentSizeTracker> sizeTrackers;
    RMLinesRenderer::Stroker<RMLinesRenderer::ClippedRaster<RMLinesRenderer::LerpRaster<rMPenFill> >,
        VaryingGeneratorLengthWidth> stroker;

    auto filtered_layers() const {
        return std::views::filter(layers, [this](const Layer &layer) -> bool {
            return std::ranges::find(
                       config.disabledLayers,
                       layer.groupId
                   ) == std::ranges::end(config.disabledLayers);
        });
    }

    auto filtered_lines(const Layer &layer) const {
        return std::views::filter(layer.lines, [this](const LayerInfo::LineInfo &line) -> bool {
            if (config.useWhitelist) {
                return std::ranges::find(
                           config.penWhitelist,
                           static_cast<int>(line.line.tool)
                       ) != std::ranges::end(config.penWhitelist);
            }

            return std::ranges::find(
                       config.penBlacklist,
                       static_cast<int>(line.line.tool)
                   ) == std::ranges::end(config.penBlacklist);
        });
    }
};
