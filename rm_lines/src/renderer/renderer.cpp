#include "renderer/renderer.h"
#include <format>

#include "advanced/text.h"

Renderer::Renderer(SceneTree *sceneTree) {
    this->sceneTree = sceneTree;
    calculateAnchors();
}

Renderer::~Renderer() {
}

void Renderer::calculateAnchors() {
    // Clear the current anchors
    anchors.clear();

    // Map special anchors
    anchors[CrdtId(0, 281474976710654)] = 270;
    anchors[CrdtId(0, 281474976710655)] = 700;

    // Check for the root text
    if (!sceneTree->rootText) return;

    // Initialize start positions
    int yOffset = TEXT_TOP_Y;
    int posX = 0, posY = 0;

    // Calculate the anchors
    sceneTree->rootText->items.expandTextItems();
    auto textIds = sceneTree->rootText->items.getSortedIds();

    std::string debugTextIds = "TextIds: ";
    for (const auto &textId: textIds) {
        debugTextIds += textId.repr() + " ";
    }
    logDebug(debugTextIds);

    for (const auto &textId: textIds) {
        auto text = sceneTree->rootText->items[textId];
        // auto [_, paragraphStyle] = sceneTree->rootText->styleMap[textId];

        logDebug(reprTextItem(text));

        // Get the height for this paragraph style
        // yOffset += lineHeights[paragraphStyle].second;
        //
        // posX += 0;
        // posY += yOffset;
    }
}
