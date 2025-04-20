#include "renderer/renderer.h"
#include <format>

#include "advanced/text.h"

Renderer::Renderer(SceneTree *sceneTree, PageType pageType, bool landscape): _sizeTracker(
    sceneTree->sceneInfo->paperSize.value_or<IntPair>({1404, 1872}), pageType) {
    this->sceneTree = sceneTree;
    this->_sizeTracker.reverseFrameSize = landscape;
    prepareTextDocument();
    calculateAnchors();
}

Renderer::~Renderer() {
}

void Renderer::prepareTextDocument() {
    if (!sceneTree->rootText) return;

    textDocument.fromText(sceneTree->rootText.value());
}

void Renderer::trackX(const float posX) {
    _sizeTracker.trackX(posX);
}

void Renderer::trackY(const float posY) {
    logDebug(std::format("TRACK[Y] {}", posY));
    _sizeTracker.trackY(posY);
}

void Renderer::calculateAnchors() {
    // Clear the current anchors
    anchors.clear();

    // Map special anchors
    anchors[ANCHOR_ID_START] = 270;
    anchors[ANCHOR_ID_END] = 700;

    // Check for the root text
    if (!sceneTree->rootText) return;

    // Initialize start positions
    int yOffset = TEXT_TOP_Y;
    int posX = 0, posY = 0;

    // Calculate the anchors
    for (const auto &paragraph: textDocument.paragraphs) {
        // Get the height for this paragraph style
        yOffset += LineHeights[paragraph.style.value].second;

        posX += 0;
        posY += yOffset;

        // Save the anchor for this paragraph
        anchors[paragraph.startId] = posY;
        trackY(posY);
    }
}
