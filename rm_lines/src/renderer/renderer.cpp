#include "renderer/renderer.h"
#include <format>

#include "advanced/text.h"
#include "renderer/image_renderer.h"
#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "renderer/rm_lines_stroker/templates/template_functions.h"
#include "writer/tagged_block_writer.h"
#define HTML_HEADER "<!DOCTYPE html><html><body>"
#define HTML_FOOTER "</body></html>"


Renderer::Renderer(SceneTree *sceneTree, const PageType pageType, const bool landscape) : paperSize({
        BASE_PAPER_SIZE_X, BASE_PAPER_SIZE_Y
    }),
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
        groupLayerItems(layer, LAYER_INFO_NODE, layer.groupId);
    }
    setTemplate("Blank");
}

void Renderer::prepareTextDocument() {
    if (!sceneTree->hasText()) return;

    textDocument.fromText(sceneTree->getText());
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
    auto it = sizeTrackers.emplace(layerId, DocumentSizeTracker(paperSize, pageType, landscape)).first;
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
    if (!sceneTree->hasText()) return;

    // Initialize start positions
    int yOffset = TEXT_TOP_Y;
    int posY = 0;

    // Calculate the anchors
    for (const auto &paragraph: textDocument.paragraphs) {
        // Get the height for this paragraph style
        const auto styleHeight = paragraph.style.value.getLineHeight();
        yOffset += styleHeight;

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

void Renderer::groupLayerItems(Layer &layer, const CrdtId parentId, const CrdtId groupId, int offsetX, int offsetY) {
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
            groupLayerItems(layer, groupId, subGroupId.value.value(), offsetX, offsetY);
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
            layer.lines.push_back(LayerInfo::LineInfo{
                .line = std::move(line.value.value()),
                .groupId = groupId,
                .itemId = line.itemId,
                .offsetX = trackX(layer.groupId, offsetX),
                .offsetY = trackY(layer.groupId, offsetY),
            });
        } else if (std::holds_alternative<CrdtSequenceItem<Image> >(node)) {
            auto image = std::get<CrdtSequenceItem<Image> >(node);
            if (!image.value.has_value()) continue;
            for (int i = 0; i < image.value->vertices.size(); i += 4) {
                auto x = image.value->vertices[i] + offsetX;
                auto y = image.value->vertices[i + 1] + offsetY;
                // ReSharper disable once CppNoDiscardExpression
                trackX(layer.groupId, x);
                trackY(layer.groupId, y);
            }
            layer.images.push_back(LayerInfo::ImageInfo{
                .image = std::move(image.value.value()),
                .groupId = groupId,
                .itemId = image.itemId,
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

json Renderer::getAnchors() const {
    json j;
    for (const auto &[id, pos]: anchors) {
        j[id.toJson()] = pos;
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

float Renderer::getTextMargin() const {
    return frameSize.halfX() - getTextWidth() / 2;
}

float Renderer::getTextWidth() const {
    // The size of the text is based on the rM2
    // We need to scale it relative to the paperSize
    const float screenRelative = frameSize.x / BASE_PAPER_SIZE_X;
    const float width = textDocument.text->width.value;
    return width * screenRelative;
}

void Renderer::toMd(std::ostream &stream) const {
    int numbered = 0;
    bool sub1 = false;
    for (const auto &paragraph: textDocument.paragraphs) {
        // Write style prefix based on paragraph style
        for (int i = 0; i < paragraph.style.value.tabbed(); i++) {
            stream << "- ";
        }
        switch (paragraph.style.value.legacy) {
            case Numbered:
            case NumberedTab:
                numbered++;
                break;
            default:
                numbered = 0;
                break;
        }
        switch (paragraph.style.value.legacy) {
            case Title:
                stream << "# ";
                sub1 = false;
                break;
            case Sub:
                if (sub1) {
                    stream << "### ";
                } else {
                    stream << "## ";
                    sub1 = true;
                }

                break;
            case Bullet:
            case BulletTab:
                stream << "- ";
                break;
            case CheckBox:
            case CheckBoxTab:
                stream << "☐ ";
                break;
            case CheckBoxChecked:
            case CheckBoxTabChecked:
                stream << "**☑** ";
                break;
            case Numbered:
            case NumberedTab:
                stream << numbered << ". ";
                break;
            case PlainText:
            default:
                break;
        }

        // Add strikethrough for checked checkboxes
        if (paragraph.style.value.legacy == CheckBoxChecked) {
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
        if (paragraph.style.value.legacy == CheckBoxChecked) {
            stream << "~~";
        }

        // Add double newline after each paragraph
        stream << "\n\n";
    }
}

void Renderer::toRM(std::ostream &stream) const {
    auto writer = TaggedBlockWriter(stream, this);
    if (!writer.buildRM()) {
        logError("Failed to build RM file!");
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

void Renderer::getFrame(uint32_t *data, const size_t dataSize, Vector position, Vector frameSize,
                        const Vector bufferSize, const bool antialias) {
    const auto buf = &stroker.raster.raster.fill.buffer;
    const auto lineBuf = &stroker.raster.raster.fill.lineBuffer;
    const auto scale = bufferSize / frameSize;

    position *= -1; // It's technically an offset

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
        for (const auto &image: layer.images) {
            stroker.raster.raster.fill.baseColor = Color(255, 0, 255, 255);
            stroker.raster.raster.fill.debugTool(3.0f);
            double startX, startY;
            for (int i = 0; i < image.image.vertices.size(); i += 4) {
                auto x = position.x + image.image.vertices[i] + image.offsetX + this->frameSize.x / 2;
                auto y = position.y + image.image.vertices[i + 1] + image.offsetY;
                x *= scale.x;
                y *= scale.y;
                if (i == 0) {
                    startX = x;
                    startY = y;
                    stroker.moveTo(x, y);
                } else {
                    stroker.lineTo(x, y);
                }
            }
            stroker.lineTo(startX, startY);
            stroker.finish();

            const auto textureIt = imageRefMap.find(image.image.imageRef.value);
            if (textureIt == imageRefMap.end() || !textureIt->second || !textureIt->second->data) {
                logError(std::format("Image texture {} not loaded", image.image.imageRef.value));
                continue;
            }

            RendererImage::renderImage(*buf, *textureIt->second, image, position, this->frameSize, scale);
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
                if (sceneTree->hasText()) {
                    stroker.moveTo((position.x + getTextMargin()) * scale.x,
                                   (position.y + anchor) * scale.y);
                    stroker.lineTo(
                        (position.x + getTextMargin() + getTextWidth()) * scale.x,
                        (position.y + anchor) * scale.y);
                } else {
                    stroker.moveTo(0, (position.y + anchor) * scale.y);
                    stroker.lineTo(buf->width, (position.y + anchor) * scale.y);
                }
                stroker.finish();
            }
        }
    }

    templateFunction(&stroker.raster.raster.fill, this);

    buf->exportRawData(data, dataSize, antialias, 1);
    stroker.raster.raster.fill.reset();
}

void Renderer::setTemplate(const std::string &templateName) {
    this->templateName = templateName;
    logDebug(std::format("Template name: {}", templateName));
    switch (hashString(templateName.c_str())) {
        case hashString("Blank"):
            templateFunction = Blank;
            break;
        case hashString("P Grid large"):
        case hashString("P Grid medium"):
        case hashString("P Grid margin med"):
        case hashString("P Grid margin large"):
        case hashString("P Grid small"):
            templateFunction = GridBase;
            break;
        default:
            logError(std::format("Unknown template name: {}", templateName));
            templateFunction = Blank;
            break;
    }
}

void Renderer::addImage(const char *uuid, const char *path) {
    auto ref = ImageRef::load(path);
    imageRefMap[uuid] = std::make_shared<ImageRef>(ref);
}
