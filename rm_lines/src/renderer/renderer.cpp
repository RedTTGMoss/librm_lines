#include "renderer/renderer.h"
#include <format>

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
    for (const auto &text: sceneTree->rootText->items.sequence | std::views::values) {
        auto [_, paragraphStyle] = sceneTree->rootText->styleMap[text.itemId];

        // Get the height for this paragraph style
        yOffset += lineHeights[paragraphStyle].second;

        posX += 0;
        posY += yOffset;
        
    }
}
