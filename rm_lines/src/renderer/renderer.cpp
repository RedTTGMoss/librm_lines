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
    auto textIds = sceneTree->rootText->items.getSortedIds();
    for (const auto &textId : textIds){
        auto text = sceneTree->rootText->items[textId];
        auto [_, paragraphStyle] = sceneTree->rootText->styleMap[textId];

        auto value = text.value.value();
        std::string valueDebug;

        if (std::holds_alternative<std::string>(value)) {
            std::string rawString = std::get<std::string>(value);
            std::ostringstream oss;

            // Replace escape sequences with their literal representations
            for (char c : rawString) {
                switch (c) {
                    case '\n': oss << "\\n"; break;
                    case '\t': oss << "\\t"; break;
                    case '\\': oss << "\\\\"; break;
                    case '\"': oss << "\\\""; break;
                    default: oss << c; break;
                }
            }

            valueDebug = "\"" + oss.str() + "\"";
        } else if (std::holds_alternative<uint32_t>(value)) {
            valueDebug = std::to_string(std::get<uint32_t>(value));
        }

        logDebug(
        std::format(
            "DEBUG Text item /w python: "
            "CrdtSequenceItem("
            "CrdtId({}, {}), "
            "CrdtId({}, {}), "
            "CrdtId({}, {}), {}, "
            "{}"
            ")", textId.first, textId.second,
            text.leftId.first, text.leftId.second,
            text.rightId.first, text.rightId.second,
            text.deletedLength,
            valueDebug
            )
        );

        // Get the height for this paragraph style
        yOffset += lineHeights[paragraphStyle].second;

        posX += 0;
        posY += yOffset;

        
    }
}
