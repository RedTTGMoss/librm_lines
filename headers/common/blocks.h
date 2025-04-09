#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <common/data_types.h>
#include <common/crdt_sequence_item.h>
#include <optional>

#include "scene_tree.h"
#include "common/scene_items.h"

enum BlockTypes {
    MIGRATION_INFO_BLOCK = 0,
    SCENE_TREE_BLOCK = 1,
    TREE_NODE_BLOCK = 2,
    SCENE_GLYPH_ITEM_BLOCK = 3,
    SCENE_GROUP_ITEM_BLOCK = 4,
    SCENE_LINE_ITEM_BLOCK = 5,
    SCENE_TEXT_ITEM_BLOCK = 6,
    ROOT_TEXT_BLOCK = 7,
    SCENE_TOMBSTONE_ITEM_BLOCK = 8,
    AUTHOR_IDS_BLOCK = 9,
    PAGE_INFO_BLOCK = 10,
    SCENE_INFO_BLOCK = 13,
};

class TaggedBlockReader;

struct BlockInfo {
    uint32_t offset;
    uint32_t size;
    uint8_t minVersion;
    uint8_t currentVersion;
    uint8_t blockType;

    BlockTypes getBlockType() const;
};

struct SubBlockInfo {
    uint32_t offset;
    uint32_t size;
};

struct Block {
    Block();

    virtual ~Block();

    virtual bool read(TaggedBlockReader *reader);

    static std::unique_ptr<Block> lookup(const BlockInfo &info);
};

struct UnreadableBlock final : public Block {
    // Represents an unreadable block
};


struct AuthorIdsBlock final : public Block {
    std::map<uint32_t, std::string> authorIds;

    bool read(TaggedBlockReader *reader) override;
};

struct MigrationInfoBlock final : public Block {
    CrdtId migrationId;
    bool isDevice;

    bool read(TaggedBlockReader *reader) override;
};

struct PageInfoBlock final : public Block {
    uint32_t loadsCount;
    uint32_t mergesCount;
    uint32_t textCharsCount;
    uint32_t textLinesCount;
    uint32_t typeFolioUseCount = 0;

    bool read(TaggedBlockReader *reader) override;
};

struct SceneInfoBlock final : public Block {
    LwwItem<CrdtId> currentLayer;
    std::optional<LwwItem<bool> > backgroundVisible;
    std::optional<LwwItem<bool> > rootDocumentVisible;
    std::optional<IntPair> paperSize;

    bool read(TaggedBlockReader *reader) override;
};

struct SceneTreeBlock final : public Block {
    CrdtId treeId;
    CrdtId nodeId;
    bool isUpdate;

    // In subblock
    CrdtId parentId;

    bool read(TaggedBlockReader *reader) override;
};

struct TreeNodeBlock final : public Block {
    Group group;

    bool read(TaggedBlockReader *reader) override;
};

struct SceneItemBlock : public Block {
    explicit SceneItemBlock(const uint8_t itemType = 0) : _itemType(itemType) {
    }

    CrdtId parentId = {};
    CrdtSequenceItem<> item = {};

    // In subblock
    bool read(TaggedBlockReader *reader) override;

private:
    uint8_t _itemType;

    virtual bool readValue(TaggedBlockReader *reader) = 0;
};

struct SceneGroupItemBlock final : public SceneItemBlock {
    SceneGroupItemBlock() : SceneItemBlock(0x02) {
    }

    CrdtSequenceItem<CrdtId> item = {};

    bool readValue(TaggedBlockReader *reader) override;
};

struct SceneLineItemBlock final : public SceneItemBlock {
    SceneLineItemBlock() : SceneItemBlock(0x03) {
    }

    uint8_t version;
    CrdtSequenceItem<Line> item = {};

    bool readValue(TaggedBlockReader *reader) override;
};

struct RootTextBlock final : public Block {
    CrdtId blockId;
    Text value;

    bool read(TaggedBlockReader *reader) override;
};

struct SceneGlyphItemBlock final : public SceneItemBlock {
    SceneGlyphItemBlock() : SceneItemBlock(0x01) {
    }

    CrdtSequenceItem<GlyphRange> item = {};

    bool readValue(TaggedBlockReader *reader) override;
};

#endif //BLOCKS_H
