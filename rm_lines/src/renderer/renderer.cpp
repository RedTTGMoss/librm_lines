#include "renderer/renderer.h"
#include <format>

#include "advanced/text.h"
#define HTML_HEADER "<!DOCTYPE html><html><body>"
#define HTML_FOOTER "</body></html>"

Renderer::Renderer(SceneTree *sceneTree, const PageType pageType, const bool landscape): _sizeTracker(
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

json Renderer::getParagraphs() const {
    json j = json::array();
    for (const auto &paragraph: textDocument.paragraphs) {
        logDebug("Paragraph to json: " + paragraph.repr());
        json a = paragraph.toJson();
        j.push_back(a);
    }
    return j;
}

void Renderer::toMd(std::ostream &stream) const {
    for (const auto &paragraph : textDocument.paragraphs) {
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
        for (const auto &formattedText : paragraph.contents) {
            std::string formatting;
            if (formattedText.formatting.bold) {
                formatting += "**";
            }
            if (formattedText.formatting.italic) {
                formatting += "*";
            }

            // Create a sanitized version of the text to comply with UTF-8
            std::string sanitizedText = formattedText.getSanitizedText();

            // Write the sanitized formatted text
            stream << formatting << sanitizedText;


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

void Renderer::toHtml(std::ostream &stream) {
    stream << HTML_HEADER;
    // TODO: Implement HTML rendering based on rM rendering, textDocument and *GliphRange(s)* for markings on the text!
    stream << HTML_FOOTER;
}
