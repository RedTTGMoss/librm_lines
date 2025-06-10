#include "renderer/renderer.h"
#include <format>

#include "advanced/text.h"
#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "renderer/rm_lines_stroker/templates/template_functions.h"
#define HTML_HEADER "<!DOCTYPE html><html><body>"
#define HTML_FOOTER "</body></html>"


Renderer::Renderer(SceneTree *sceneTree, const PageType pageType, const bool landscape): paperSize({1404, 1872}),
    landscape(landscape), pageType(pageType),
    sceneTree(sceneTree) {
    // Check for new paperSize in scene info block if applicable
    if (sceneTree->sceneInfo.has_value() && sceneTree->sceneInfo.value().paperSize.has_value()) {
        this->paperSize = sceneTree->sceneInfo.value().paperSize.value();
    }
    this->frameSize = landscape
                          ? Vector(paperSize.second, paperSize.first)
                          : Vector(paperSize.first, paperSize.second);

    initSizeTracker(TEXT_LAYER);

    layers = layersFromSceneTree(sceneTree);

    prepareTextDocument();
    calculateAnchors();

    for (auto &layer: layers) {
        initSizeTracker(layer.groupId);
        groupLines(layer, LAYER_INFO_NODE, layer.groupId);
    }
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
    auto [it, inserted] = sizeTrackers.emplace(layerId, DocumentSizeTracker(paperSize, pageType, landscape));
    return &it->second;
}

auto Renderer::trackX(const CrdtId &layerId, const float posX) {
    return getSizeTracker(layerId)->trackX(posX);
}

auto Renderer::trackY(const CrdtId &layerId, const float posY) {
    return getSizeTracker(layerId)->trackY(posY);
}

void Renderer::calculateAnchors() {
    // Clear the current anchors
    anchors.clear();

    // Map special anchors
    anchors[ANCHOR_ID_START] = TEXT_TOP_Y; // You expect this to be 0 but actually the text area starts a bit lower
    anchors[ANCHOR_ID_END] = paperSize.second;

    // Check for the root text
    if (!sceneTree->rootText) return;

    // Initialize start positions
    int yOffset = TEXT_TOP_Y;
    int posX = 0, posY = 0;

    // Calculate the anchors
    for (const auto &paragraph: textDocument.paragraphs) {
        // Get the height for this paragraph style
        auto styleHeight = static_cast<float>(LineHeights[paragraph.style.value].second);
        yOffset += styleHeight;

        posX += 0;
        posY = yOffset;

        // Save the anchor for this paragraph
        anchors[paragraph.startId] = posY;
        // logDebug(std::format("Anchor for paragraph {}: {}", paragraph.startId.repr(), posY));
        for (const auto &formattedText: paragraph.contents) {
            for (const auto &characterId: formattedText.characterIDs) {
                anchors[characterId] = posY;
            }
        }
        // ReSharper disable once CppNoDiscardExpression
        trackY(TEXT_LAYER, posY);
    }
}

void Renderer::groupLines(Layer &layer, const CrdtId parentId, const CrdtId groupId, int offsetX, int offsetY) {
    const auto nodes = sceneTree->getGroupChildren(groupId);
    if (const auto group = sceneTree->getNode(groupId); group->anchorId.has_value()) {
        offsetX += group->anchorOriginX.value().value;
        if (!anchors.contains(group->anchorId.value().value)) {
            logError(std::format("need anchor id {}", group->anchorId.value().value.repr()));
            throw std::runtime_error("fix this file first");
        }
        offsetY = anchors[group->anchorId.value().value];
    }
    for (const auto &node: nodes) {
        if (std::holds_alternative<CrdtSequenceItem<CrdtId> >(node)) {
            const auto subGroupId = std::get<CrdtSequenceItem<CrdtId> >(node);
            if (!subGroupId.value.has_value()) continue;
            groupLines(layer, groupId, subGroupId.value.value(), offsetX, offsetY);
        } else if (IS_LIKELY(std::holds_alternative<CrdtSequenceItem<Line>>(node))) {
            auto line = std::get<CrdtSequenceItem<Line> >(node);
            if (!line.value.has_value()) continue;
            for (const auto point: line.value.value().points) {
                auto x = point.x + offsetX;
                auto y = point.y + offsetY;
                // ReSharper disable once CppNoDiscardExpression
                trackX(layer.groupId, x);
                trackY(layer.groupId, y);
            }
            layer.lines.push_back(LineInfo{
                .line = std::move(line.value.value()),
                .groupId = groupId,
                .offsetX = trackX(layer.groupId, offsetX),
                .offsetY = trackY(layer.groupId, offsetY),
            });
        }
    }
    auto sizeTracker = getSizeTracker(layer.groupId);
    // Layer size tracker debug
    // logDebug(std::format(
    //     "Size tracker for layer {}: FS: {}x{} TR: {}->{}x{}->{}",
    //     layer.groupId.repr(),
    //     sizeTracker->getFrameWidth(), sizeTracker->getFrameHeight(),
    //     sizeTracker->getLeft(), sizeTracker->getRight(),
    //     sizeTracker->getTop(), sizeTracker->getBottom()
    // ));
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
            case HEADING:
                stream << "# ";
                break;
            case BOLD:
                stream << "## ";
                break;
            case BULLET:
            case BULLET2:
                stream << "- ";
                break;
            case CHECKBOX:
                stream << "☐ ";
                break;
            case CHECKBOX_CHECKED:
                stream << "**☑** ";
                break;
            case PLAIN:
            default:
                break;
        }

        // Add strikethrough for checked checkboxes
        if (paragraph.style.value == CHECKBOX_CHECKED) {
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
        if (paragraph.style.value == CHECKBOX_CHECKED) {
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
    // TODO: Implement HTML rendering based on rM rendering, textDocument and *GlyphRange(s)* for markings on the text!
    stream << HTML_FOOTER;
}

void Renderer::getFrame(uint32_t *data, const size_t dataSize, Vector position, const Vector frameSize,
                        const Vector bufferSize, const bool antialias) {
    const auto buf = &stroker.raster.raster.fill.buffer;
    const auto lineBuf = &stroker.raster.raster.fill.lineBuffer;
    const auto scale = bufferSize / frameSize;

    // Align the position to scale properly to the center of the requested frame
    // The scale is automatically calculated by the provided bufferSize and frameSize
    // In this case the frameSize could be let's say smaller but the buffer is larger and the scale is 2
    // To compensate for this we need to align the position to the center of the frame
    // If this isn't done the scaling will look as if from the top left corner
    position.x = (position.x + frameSize.x / 2) * scale.x;
    position.y = (position.y + frameSize.y / 2) * scale.y;

    stroker.joinStyle = JoinStyle::RoundJoin;
    stroker.raster.raster.fill.stroker = &stroker;
    // ReSharper disable once CppDFALocalValueEscapesFunction
    stroker.raster.raster.fill.position = &position;
    stroker.raster.raster.fill.scale = std::min(scale.x, scale.y);
    buf->allocate(bufferSize);
    buf->fill(0x00FFFFFF);
    lineBuf->allocate(bufferSize);
    stroker.raster.x1 = static_cast<float>(buf->width);
    stroker.raster.y1 = static_cast<float>(buf->height);

    templateFunction(&stroker.raster.raster.fill);
    // TODO: render the text

    for (const auto &layer: layers) {
        // For each layer, each line, each point
        for (const auto &line: layer.lines) {
            bool first = true;
            stroker.raster.raster.fill.line = &line.line;
            stroker.raster.raster.fill.newLine();
            for (const auto &point: line.line.points) {
                stroker.raster.raster.fill.point = &point;
                stroker.raster.raster.fill.newPoint();
                // This shouldn't need to have added half width, what's going on here?
                // We need to make sure this works when the document has expansions
                // I can confirm it needs to be paperSize and not the expanded document size tracker size!
                auto x = position.x + point.x + line.offsetX + this->frameSize.x / 2;
                auto y = position.y + point.y + line.offsetY;
                // TODO: Maybe apply a centered scale instead? Scaling the entire document isn't quite right
                x *= scale.x;
                y *= scale.y;

                if (first) {
                    stroker.moveTo(x, y);
                    first = false;
                } else {
                    stroker.lineTo(x, y);
                }
            }
            stroker.finish();
        }
        if (getDebugMode()) {
            const DocumentSizeTracker *sizeTracker = getSizeTracker(layer.groupId);
            auto x = static_cast<size_t>(sizeTracker->getFrameWidth()) / 2;
            auto y = static_cast<size_t>(sizeTracker->getFrameHeight()) / 2;

            x += position.x;
            y += position.y;
            x *= scale.x;
            y *= scale.y;

            uint32_t width, height;

            if (landscape) {
                // Landscape mode, swap x and y
                width = paperSize.second;
                height = paperSize.first;
            } else {
                width = paperSize.first;
                height = paperSize.second;
            }

            auto x2 = static_cast<size_t>(width) / 2;
            auto y2 = static_cast<size_t>(height) / 2;

            x2 += position.x;
            y2 += position.y;
            x2 *= scale.x;
            y2 *= scale.y;

            auto left = position.x * scale.x, top = position.y * scale.y;
            auto right = (position.x + width) * scale.x, bottom =
                    (position.y + height) * scale.y;


            // Make basic test tool and a point

            stroker.raster.raster.fill.baseColor = Color(150, 0, 150, 255);
            stroker.raster.raster.fill.debugTool(5.0f);

            // Draw a rect and cross of the frame
            stroker.moveTo(left, top);
            stroker.lineTo(right, top);
            stroker.lineTo(right, bottom);
            stroker.lineTo(left, bottom);
            stroker.lineTo(left, top);
            stroker.lineTo(right, bottom);
            stroker.moveTo(right, top);
            stroker.lineTo(left, bottom);
            stroker.finish();

            stroker.raster.raster.fill.baseColor = Color(0, 150, 150);
            top = (position.y + sizeTracker->getTop()) * scale.y;
            bottom = (position.y + sizeTracker->getBottom()) * scale.y;
            left = (position.x + sizeTracker->getLeft()) * scale.x;
            right = (position.x + sizeTracker->getRight()) * scale.x;

            stroker.moveTo(left, top);
            stroker.lineTo(right, top);
            stroker.lineTo(right, bottom);
            stroker.lineTo(left, bottom);
            stroker.lineTo(left, top);
            stroker.lineTo(right, bottom);
            stroker.moveTo(right, top);
            stroker.lineTo(left, bottom);
            stroker.finish();

            stroker.raster.raster.fill.baseColor = Color(150, 0, 0);
            stroker.moveTo(x, 0);
            stroker.lineTo(x, buf->height);
            stroker.moveTo(0, y);
            stroker.lineTo(buf->width, y);
            stroker.finish();

            stroker.raster.raster.fill.baseColor = Color(0, 150, 0);
            stroker.moveTo(x2, 0);
            stroker.lineTo(x2, buf->height);
            stroker.moveTo(0, y2);
            stroker.lineTo(buf->width, y2);
            stroker.finish();

            bool alternate = true;
            for (auto anchor: anchors | std::views::values) {
                if (alternate)
                    stroker.raster.raster.fill.baseColor = Color(150, 200, 0, 200);
                else
                    stroker.raster.raster.fill.baseColor = Color(150, 150, 255, 200);
                alternate = !alternate;
                stroker.moveTo(0, (position.y + anchor) * scale.y);
                stroker.lineTo(buf->width, (position.y + anchor) * scale.y);
                stroker.finish();
            }
        }
    }


    buf->exportRawData(data, dataSize, antialias, 1);
    stroker.raster.raster.fill.reset();
}

void Renderer::setTemplate(const std::string &templateName) {
    if (templateName == "Blank") {
        templateFunction = Blank;
    } else {
        templateFunction = Blank;
    }
}
