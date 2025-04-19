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
    logDebug(textDocument.repr());
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

    // std::string debugTextIds = "TextIds: ";
    // for (const auto &textId: textIds) {
    //     debugTextIds += textId.repr() + " ";
    // }
    // logDebug(debugTextIds);
    //
    // for (const auto &textId: textIds) {
    //     auto text = sceneTree->rootText->items[textId];
    //     // auto [_, paragraphStyle] = sceneTree->rootText->styleMap[textId];
    //
    //     logDebug(reprTextItem(text));
    //
    //     // Get the height for this paragraph style
    //     // yOffset += lineHeights[paragraphStyle].second;
    //     //
    //     // posX += 0;
    //     // posY += yOffset;
    // }
}
