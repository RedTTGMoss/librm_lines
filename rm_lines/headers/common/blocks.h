#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <common/data_types.h>
#include <common/crdt_sequence_item.h>
#include <optional>

#include "common/scene_items.h"

#include <nlohmann/json.hpp>

class TaggedBlockWriter;
using json = nlohmann::json;

enum BlockTypes {
    UNREADABLE_BLOCK = -1,
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
    IMAGE_INFO_BLOCK = 14,
    SCENE_IMAGE_ITEM_BLOCK = 15,
};

class TaggedBlockReader;

struct BlockInfo {
    uint32_t offset;
    uint32_t size;
    uint8_t minVersion;
    uint8_t currentVersion;
    uint8_t blockType;

    [[nodiscard]] BlockTypes getBlockType() const;
};

struct SubBlockInfo {
    uint32_t offset;
    uint32_t size;
};

struct Block {
    std::optional<BlockInfo> info = std::nullopt;

    Block();

    virtual ~Block();

    virtual bool read(TaggedBlockReader *reader);

    virtual bool write(TaggedBlockWriter *writer) const;

    virtual BlockTypes getBlockType() const;

    static std::unique_ptr<Block> lookup(const BlockInfo &info);

    [[nodiscard]] virtual json toJson() const = 0;
};

struct UnreadableBlock final : Block {
    // Represents an unreadable block
    [[nodiscard]] json toJson() const override {
        return {};
    }
};

struct SceneTombstoneItemBlock final : Block {
    CrdtId itemId;

    bool read(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return SCENE_TOMBSTONE_ITEM_BLOCK;
    }
};

struct AuthorIdsBlock final : Block {
    std::map<uint16_t, std::string> authorIds;

    bool read(TaggedBlockReader *reader) override;

    bool write(TaggedBlockWriter *writer) const override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return AUTHOR_IDS_BLOCK;
    }
};

struct MigrationInfoBlock final : Block {
    CrdtId migrationId;
    bool isDevice;
    std::optional<bool> isV3 = std::nullopt;

    bool read(TaggedBlockReader *reader) override;

    bool write(TaggedBlockWriter *writer) const override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return MIGRATION_INFO_BLOCK;
    }
};

struct PageInfoBlock final : Block {
    uint32_t loadsCount;
    uint32_t mergesCount;
    uint32_t textCharsCount;
    uint32_t textLinesCount;
    uint32_t typeFolioUseCount = 0;

    bool read(TaggedBlockReader *reader) override;

    bool write(TaggedBlockWriter *writer) const override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return PAGE_INFO_BLOCK;
    }
};

struct SceneInfoBlock final : Block {
    LwwItem<CrdtId> currentLayer;
    std::optional<LwwItem<bool> > backgroundVisible;
    std::optional<LwwItem<bool> > rootDocumentVisible;
    std::optional<IntPair> paperSize;
    std::optional<LwwItem<RectPair> > extendedContentRect;
    std::optional<DoublePair> paperSizeF;
    std::optional<LwwItem<DoublePair> > lwwPaperSizeF;
    std::optional<LwwItem<uint8_t> > preferredLayout;

    bool read(TaggedBlockReader *reader) override;

    bool write(TaggedBlockWriter *writer) const override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return SCENE_INFO_BLOCK;
    }
};

struct SceneTreeBlock final : Block {
    CrdtId treeId;
    CrdtId nodeId;
    bool isUpdate;

    // In subblock
    CrdtId parentId;

    bool read(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return SCENE_TREE_BLOCK;
    }
};

struct TreeNodeBlock final : Block {
    Group group;

    bool read(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return TREE_NODE_BLOCK;
    }
};

struct SceneItemBlock : Block {
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

struct SceneGroupItemBlock final : SceneItemBlock {
    SceneGroupItemBlock() : SceneItemBlock(0x02) {
    }

    CrdtSequenceItem<CrdtId> item = {};

    bool readValue(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return SCENE_GROUP_ITEM_BLOCK;
    }
};

struct SceneLineItemBlock final : SceneItemBlock {
    SceneLineItemBlock() : SceneItemBlock(0x03) {
    }

    uint8_t version = 0;
    CrdtSequenceItem<Line> item = {};

    bool readValue(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return SCENE_LINE_ITEM_BLOCK;
    }
};

struct RootTextBlock final : Block {
    CrdtId blockId;
    Text value;

    bool read(TaggedBlockReader *reader) override;

    json toJson() const override;

    BlockTypes getBlockType() const override {
        return ROOT_TEXT_BLOCK;
    }
};

struct SceneGlyphItemBlock final : SceneItemBlock {
    SceneGlyphItemBlock() : SceneItemBlock(0x01) {
    }

    CrdtSequenceItem<GlyphRange> item = {};

    bool readValue(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return SCENE_GLYPH_ITEM_BLOCK;
    }
};

struct ImageInfoBlock final : Block {
    std::vector<ImageInfo> images;

    bool read(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return IMAGE_INFO_BLOCK;
    }
};

struct SceneImageItemBlock final : SceneItemBlock {
    SceneImageItemBlock() : SceneItemBlock(0x07) {
    }

    CrdtSequenceItem<ImageItem> item = {};

    bool readValue(TaggedBlockReader *reader) override;

    [[nodiscard]] json toJson() const override;

    BlockTypes getBlockType() const override {
        return SCENE_IMAGE_ITEM_BLOCK;
    }
};
