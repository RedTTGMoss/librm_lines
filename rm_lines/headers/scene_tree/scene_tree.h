#pragma once

#include <unordered_map>


#include <memory>
#include <common/data_types.h>

#include <variant>
#include <common/data_types.h>
#include <common/crdt_sequence_item.h>
#include <common/scene_items.h>
#include <nlohmann/json.hpp>

#include "../common/blocks.h"

using json = nlohmann::json;


using SceneItemVariant = std::variant<
    CrdtSequenceItem<Group>,
    CrdtSequenceItem<CrdtId>,
    CrdtSequenceItem<GlyphRange>,
    CrdtSequenceItem<Line>,
    CrdtSequenceItem<Text>,
    CrdtSequenceItem<Image>
>;

static CrdtId &getItemId(SceneItemVariant &item) {
    return std::visit([](auto &x) -> CrdtId & {
        return x.itemId;
    }, item);
}

static const CrdtId &getItemId(const SceneItemVariant &item) {
    return std::visit([](const auto &x) -> const CrdtId & {
        return x.itemId;
    }, item);
}

class SceneTree {
public:
    SceneTree();

    std::optional<AuthorIdsBlock> authorsInfo;
    std::optional<MigrationInfoBlock> migrationInfo;
    std::optional<PageInfoBlock> pageInfo;
    std::optional<SceneInfoBlock> sceneInfo;
    std::optional<ImageInfoBlock> imageInfo;

    void addNode(const CrdtId &nodeId, const CrdtId &parentNodeId, const CrdtId &parentTreeId);

    void addItem(const SceneItemVariant &item, const CrdtId &parentId);

    Group *getNode(const CrdtId &nodeId);

    std::vector<SceneItemVariant> getGroupChildren(const CrdtId &nodeId);

    json toJson();

    friend class TaggedBlockWriter;
    friend class SceneTreeEditor;

    void setText(Text &&text) {
        rootText = std::make_shared<Text>(std::move(text));
    }

    void clearText() {
        rootText.reset();
    }

    bool hasText() const {
        return static_cast<bool>(rootText);
    }

    std::shared_ptr<Text> getText() const {
        return rootText;
    }

private:
    std::shared_ptr<Text> rootText;
    std::map<CrdtId, std::unique_ptr<Group> > _nodeIds;
    std::map<CrdtId, std::vector<SceneItemVariant> > _groupChildren;
};
