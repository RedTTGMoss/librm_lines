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
    CrdtSequenceItem<ImageItem>
>;

class SceneTree {
public:
    SceneTree();

    std::optional<AuthorIdsBlock> authorsInfo;
    std::optional<MigrationInfoBlock> migrationInfo;
    std::optional<PageInfoBlock> pageInfo;
    std::optional<SceneInfoBlock> sceneInfo;
    std::optional<ImageInfoBlock> imageInfo;
    std::optional<Text> rootText;

    void addNode(const CrdtId &nodeId, const CrdtId &parentNodeId, const CrdtId &parentTreeId);

    void addItem(const SceneItemVariant &item, const CrdtId &parentId);

    Group *getNode(const CrdtId &nodeId);

    std::vector<SceneItemVariant> getGroupChildren(const CrdtId &nodeId);

    json toJson();

    friend class TaggedBlockWriter;

private:
    std::map<CrdtId, std::unique_ptr<Group> > _nodeIds;
    std::map<CrdtId, std::vector<SceneItemVariant> > _groupChildren;
};
