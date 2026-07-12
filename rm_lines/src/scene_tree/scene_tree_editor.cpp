#include "scene_tree/scene_tree_editor.h"

#include "advanced/text.h"
#include "advanced/text_scale.h"
#include "scene_tree/scene_tree_export.h"

CrdtId SceneTreeEditor::createLayer(const std::string &label) {
    const auto layerId = ids;
    const auto node = createSceneTree(layerId, label);
    addSceneTree(std::move(node));
    _layers.push_back(layerId);
    return layerId;
}

Group SceneTreeEditor::createSceneTree(const CrdtId &id, const std::string &label) {
    Group group;
    group.parentId = ROOT_NODE;
    group.updated = true; // This ensures it will be written
    ids = id;
    group.nodeId = id;
    if (!label.empty()) {
        ids++;
        group.label.timestamp = id + 1;
        group.label.value = label;
    }
    return group;
}

CrdtId SceneTreeEditor::addSceneTree(const Group &&node) {
    _nodeIds[node.nodeId] = std::make_unique<Group>(std::move(node));
    _nodeIds[node.nodeId].get()->parentIs = TREE;

    CrdtSequenceItem<CrdtId> childItem;
    childItem.itemId = ids++;
    childItem.value = node.nodeId;
    addItem(childItem, node.parentId);
    return childItem.itemId;
}

CrdtId SceneTreeEditor::addItemNode(SceneItemVariant item) {
    CrdtId itemId = END_MARKER;
    std::visit([&](auto &typedItem) {
        if (typedItem.itemId != END_MARKER) {
            itemId = typedItem.itemId;
        } else {
            itemId = ids++;
        }
        typedItem.itemId = itemId;

        if (!_groupChildren[currentLayer].empty()) {
            typedItem.leftId = getItemId(_groupChildren[currentLayer].back());
        }
    }, item);
    addItem(item, currentLayer);
    return itemId;
}

void SceneTreeEditor::init() {
    // Initialize authorIDs
    authorsInfo = AuthorIdsBlock();

    // Initialize migration info
    migrationInfo = MigrationInfoBlock();

    // Initialize Layer 1
    createLayer("Layer 1");
    ids.first++;

    // Initialize SceneInfo
    sceneInfo = SceneInfoBlock::newBlock();
}

void SceneTreeEditor::initText() {
    if (!hasText()) {
        rootText = std::make_shared<Text>();
        rootText->items = TextSequence();
        setRootTextWidth(ColumnMedium);
    }
    if (!text) {
        const auto rootText = getText();
        text = new TextBuilder(rootText);
    }
}

void SceneTreeEditor::initImageInfoBlock() {
    if (!imageInfo) {
        imageInfo = ImageInfoBlock::newBlock();
    }
}

LineBuilder SceneTreeEditor::startLine() {
    return LineBuilder(this);
}

std::string SceneTreeEditor::addImageInfo(std::string filename, const std::string &uuid) {
    initImageInfoBlock();
    ImageRecordInfo info = {uuid, {ids++, filename}};
    imageInfo->images.push_back(info);
    return info.uuid;
}

void SceneTreeEditor::setRootTextWidth(TextColumnWidth width) {
    if (!hasText()) {
        throw std::runtime_error("Cannot set root text width: no root text exists");
    }
    const TextAreaInfo info = getTextAreaInfo(sceneInfo->paperSize.value(), width);
    rootText->width = LwwItem(ids++, info.width);
    rootText->posX = info.x;
    rootText->posY = info.y;
}

CrdtId SceneTreeEditor::addImage(const std::string &uuid, std::vector<AdvancedMath::Rect> vertices) {
    if (vertices.size() != 4) {
        throw std::runtime_error("Invalid number of vertices");
    }
    CrdtSequenceItem<Image> imageItem;
    imageItem.itemId = ids++;
    imageItem.value = Image();
    imageItem.value->boundsTimestamp = ids++;
    imageItem.value->imageRef.timestamp = ids++;
    imageItem.value->imageRef.value = uuid;
    imageItem.value->vertices = {
        static_cast<float>(vertices[0].x), static_cast<float>(vertices[0].y), static_cast<float>(vertices[0].w),
        static_cast<float>(vertices[0].h),
        static_cast<float>(vertices[1].x), static_cast<float>(vertices[1].y), static_cast<float>(vertices[1].w),
        static_cast<float>(vertices[1].h),
        static_cast<float>(vertices[2].x), static_cast<float>(vertices[2].y), static_cast<float>(vertices[2].w),
        static_cast<float>(vertices[2].h),
        static_cast<float>(vertices[3].x), static_cast<float>(vertices[3].y), static_cast<float>(vertices[3].w),
        static_cast<float>(vertices[3].h)
    };

    addItemNode(imageItem);

    return imageItem.itemId;
}

CrdtId SceneTreeEditor::addImage(const std::string &uuid, const std::vector<AdvancedMath::Vector> &vertices) {
    if (vertices.size() != 4) {
        throw std::runtime_error("Invalid number of vertices");
    }
    std::vector<AdvancedMath::Rect> rects;
    rects.push_back(AdvancedMath::Rect{vertices[0].x, vertices[0].y, 0, 0});
    rects.push_back(AdvancedMath::Rect{vertices[1].x, vertices[1].y, 1, 0});
    rects.push_back(AdvancedMath::Rect{vertices[2].x, vertices[2].y, 1, 1});
    rects.push_back(AdvancedMath::Rect{vertices[3].x, vertices[3].y, 0, 1});
    return addImage(uuid, rects);
}

LineBuilder::LineBuilder(SceneTreeEditor *editor, const PenTool tool, const PenColor color)
    : editor(editor) {
    nodeId = editor->ids++;
    this->tool = tool;
    this->color = color;
}

LineBuilder &LineBuilder::addPoint(const float x, const float y) {
    points.push_back({toSpaceX(x), toSpaceY(y), pointSpeed, pointDirection, pointWidth, pointPressure});
    return *this;
}

LineBuilder &LineBuilder::addPoint(const float x, const float y, const uint32_t speed, const uint32_t direction,
                                   const uint32_t width, const uint32_t pressure) {
    points.push_back({toSpaceX(x), toSpaceY(y), speed, direction, width, pressure});
    return *this;
}

LineBuilder &LineBuilder::setRGBA(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
    color = ARGB;
    argbColor = Color{r, g, b, a};
    return *this;
}

LineBuilder &LineBuilder::setRGBA(const Color &color) {
    this->color = ARGB;
    argbColor = color;
    return *this;
}

LineBuilder &LineBuilder::setPen(const PenTool tool) {
    this->tool = tool;
    return *this;
}

LineBuilder &LineBuilder::setColor(const PenColor color) {
    this->color = color;
    return *this;
}

LineBuilder &LineBuilder::usePaperSpace() {
    usingPaperSpace = true;
    return *this;
}

LineBuilder &LineBuilder::useCoordinateSpace() {
    usingPaperSpace = false;
    return *this;
}

LineBuilder &LineBuilder::setSpeed(const uint32_t speed) {
    pointSpeed = speed;
    return *this;
}

LineBuilder &LineBuilder::setDirection(const uint32_t direction) {
    pointDirection = direction;
    return *this;
}

LineBuilder &LineBuilder::setWidth(const uint32_t width) {
    pointWidth = width;
    return *this;
}

LineBuilder &LineBuilder::setPressure(const uint32_t pressure) {
    pointPressure = pressure;
    return *this;
}


CrdtId LineBuilder::endLine() {
    // assignDirections();
    if (color == ARGB && argbColor->alpha == 0) return END_MARKER;

    CrdtSequenceItem<Line> lineItem;
    lineItem.itemId = nodeId;
    lineItem.value = *this;
    editor->addItemNode(lineItem);

    return nodeId;
}

uint32_t LineBuilder::calculateDirection(const Point &prev, const float x2, const float y2) {
    const float dx = prev.x - x2;
    const float dy = prev.y - y2;

    const float angle = std::atan2(dy, dx);

    return static_cast<uint32_t>(255.0 * angle / (PI * 2));
}

TextBuilder::TextBuilder(const std::shared_ptr<Text> &_text) : text(_text) {
    textDocument.fromText(text);
}
