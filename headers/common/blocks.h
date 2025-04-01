#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <common/data_types.h>
#include <optional>

#include "common/scene_items.h"

class TaggedBlockReader;

struct BlockInfo {
    uint32_t offset;
    uint32_t size;
    uint8_t minVersion;
    uint8_t currentVersion;
    uint8_t blockType;
};

struct SubBlockInfo {
    uint32_t offset;
    uint32_t size;
};

struct Block {
    Block();

    virtual ~Block();

    virtual bool read(TaggedBlockReader *reader);

    static void lookup(Block *&block, const BlockInfo &info);
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
    std::optional<LwwItem<bool>> backgroundVisible;
    std::optional<LwwItem<bool>> rootDocumentVisible;
    std::optional<IntPair> paperSize;
    bool read(TaggedBlockReader *reader) override;
};

struct SceneTreeBlock final : public Block {
    CrdtId sceneId;
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
    SceneItemBlock(uint8_t itemType = 0) : _itemType(itemType) {}
    CrdtId parentId;
    CrdtSequenceItem<> item;

    // In subblock
    bool read(TaggedBlockReader *reader) override;
private:
    uint8_t _itemType;
    virtual bool readValue(TaggedBlockReader *reader) = 0;
};

struct SceneGroupItemBlock final : public SceneItemBlock {
    SceneGroupItemBlock() : SceneItemBlock(0x02) {}
    CrdtSequenceItem<CrdtId> item;

    bool readValue(TaggedBlockReader *reader) override;
};

struct SceneLineItemBlock final : public SceneItemBlock {
    SceneLineItemBlock() : SceneItemBlock(0x03) {}

    uint8_t version;
    CrdtSequenceItem<Line> item;

    bool readValue(TaggedBlockReader *reader) override;
};

struct RootTextBlock final : public Block {
    CrdtId blockId;
    Text value;

    bool read(TaggedBlockReader *reader) override;
};

#endif //BLOCKS_H
