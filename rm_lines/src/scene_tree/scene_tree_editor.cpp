#include "scene_tree/scene_tree_editor.h"

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

void SceneTreeEditor::addSceneTree(const Group &&node) {
    _nodeIds[node.nodeId] = std::make_unique<Group>(std::move(node));
    _nodeIds[node.nodeId].get()->parentIs = TREE;

    CrdtSequenceItem<CrdtId> childItem;
    childItem.itemId = ids++;
    childItem.value = node.nodeId;
    addItem(childItem, node.parentId);
}

void SceneTreeEditor::addItemNode(SceneItemVariant item) {
    std::visit([&](auto &typedItem) {
        typedItem.itemId = ids++;

        if (!_groupChildren[currentLayer].empty()) {
            typedItem.leftId = getItemId(_groupChildren[currentLayer].back());
        }
    }, item);
    addItem(item, currentLayer);
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
    sceneInfo = SceneInfoBlock();
}

void SceneTreeEditor::initText() {
    // TODO: Implement initializing text
}

LineBuilder SceneTreeEditor::startLine() {
    return LineBuilder(this);
}

LineBuilder::LineBuilder(SceneTreeEditor *editor, const PenTool tool, const PenColor color)
    : editor(editor) {
    nodeId = editor->ids++;
    this->tool = tool;
    this->color = color;
}

LineBuilder &LineBuilder::addPoint(const float x, const float y) {
    points.push_back({x, y, pointSpeed, pointDirection, pointWidth, pointPressure});
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


void LineBuilder::endLine() {
    // assignDirections();
    if (color == ARGB && argbColor->alpha == 0) return;

    CrdtSequenceItem<Line> lineItem;
    lineItem.itemId = nodeId;
    lineItem.value = *this;
    editor->addItemNode(lineItem);
}

uint32_t LineBuilder::calculateDirection(const Point &prev, const float x2, const float y2) {
    const float dx = prev.x - x2;
    const float dy = prev.y - y2;

    const float angle = std::atan2(dy, dx);

    return static_cast<uint32_t>(255.0 * angle / (PI * 2));
}
