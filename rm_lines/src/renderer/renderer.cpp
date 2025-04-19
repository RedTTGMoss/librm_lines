#include "renderer/renderer.h"
#include <format>

#include "advanced/text.h"

Renderer::Renderer(SceneTree *sceneTree) {
    this->sceneTree = sceneTree;
    prepareTextDocument();
    calculateAnchors();
}

Renderer::~Renderer() {
}

void Renderer::prepareTextDocument() {
    if (!sceneTree->rootText) return;

    textDocument.fromText(sceneTree->rootText.value());
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
    }
}
