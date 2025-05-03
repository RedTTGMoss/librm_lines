#include "renderer/renderer.h"
#include <format>

#include "advanced/text.h"
#define HTML_HEADER "<!DOCTYPE html><html><body>"
#define HTML_FOOTER "</body></html>"

Renderer::Renderer(SceneTree *sceneTree, const PageType pageType, const bool landscape): sceneTree(sceneTree),
    paperSize(sceneTree->sceneInfo->paperSize.value_or<IntPair>({1404, 1872})), landscape(landscape),
    pageType(pageType) {

    initSizeTracker(TEXT_LAYER);

    layers = layersFromSceneTree(sceneTree);

    prepareTextDocument();
    calculateAnchors();
}

Renderer::~Renderer() {
}

void Renderer::prepareTextDocument() {
    if (!sceneTree->rootText) return;

    textDocument.fromText(sceneTree->rootText.value());
}

DocumentSizeTracker *Renderer::getSizeTracker(const CrdtId layerId) {
    // Check if the size tracker exists
    const auto it = sizeTrackers.find(layerId);
    if (it != sizeTrackers.end()) {
        return &it->second;
    }
    // If it doesn't exist, raise an error
    throw std::runtime_error(std::format("Document size tracker for layer {} not found", layerId.repr()));
}

DocumentSizeTracker *Renderer::initSizeTracker(CrdtId layerId) {
    auto [it, inserted] = sizeTrackers.emplace(layerId, DocumentSizeTracker(paperSize, pageType));
    it->second.reverseFrameSize = landscape;
    return &it->second;
}

void Renderer::trackX(const CrdtId &layerId, const float posX) {
    getSizeTracker(layerId)->trackX(posX);
}

void Renderer::trackY(const CrdtId &layerId, const float posY) {
    getSizeTracker(layerId)->trackY(posY);
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
        trackY(TEXT_LAYER, posY);
    }
}

void Renderer::groupLines() {
}

json Renderer::getParagraphs() const {
    json j = json::array();
    for (const auto &paragraph: textDocument.paragraphs) {
        j.push_back(paragraph.toJson());
    }
    return j;
}

json Renderer::getLayers() const {
    json j = json::array();
    for (const auto &layer: layers) {
        j.push_back(layer.toJson());
    }
    return j;
}

void Renderer::toMd(std::ostream &stream) const {
    for (const auto &paragraph: textDocument.paragraphs) {
        // Write style prefix based on paragraph style
        switch (paragraph.style.value) {
            case ParagraphStyle::HEADING:
                stream << "# ";
                break;
            case ParagraphStyle::BOLD:
                stream << "## ";
                break;
            case ParagraphStyle::BULLET:
            case ParagraphStyle::BULLET2:
                stream << "- ";
                break;
            case ParagraphStyle::CHECKBOX:
                stream << "☐ ";
                break;
            case ParagraphStyle::CHECKBOX_CHECKED:
                stream << "**☑** ";
                break;
            case ParagraphStyle::PLAIN:
            default:
                break;
        }

        // Add strikethrough for checked checkboxes
        if (paragraph.style.value == ParagraphStyle::CHECKBOX_CHECKED) {
            stream << "~~";
        }

        // Process each formatted text segment
        for (const auto &formattedText: paragraph.contents) {
            std::string formatting;
            if (formattedText.formatting.bold) {
                formatting += "**";
            }
            if (formattedText.formatting.italic) {
                formatting += "*";
            }

            // Create a sanitized version of the text to comply with UTF-8

            // Write the sanitized formatted text
            stream << formatting << formattedText.text;


            // Close formatting tags in reverse order
            std::string reverseFormatting(formatting.rbegin(), formatting.rend());
            stream << reverseFormatting;
        }

        // Close strikethrough if needed
        if (paragraph.style.value == ParagraphStyle::CHECKBOX_CHECKED) {
            stream << "~~";
        }

        // Add double newline after each paragraph
        stream << "\n\n";
    }
}

void Renderer::toTxt(std::ostream &stream) const {
    for (const auto &paragraph: textDocument.paragraphs) {
        for (const auto &formattedText: paragraph.contents) {
            // Write the formatted text
            stream << formattedText.text;
        }
        stream << "\n"; // Add newline after each paragraph
    }
}

void Renderer::toHtml(std::ostream &stream) {
    stream << HTML_HEADER;
    // TODO: Implement HTML rendering based on rM rendering, textDocument and *GliphRange(s)* for markings on the text!
    stream << HTML_FOOTER;
}
