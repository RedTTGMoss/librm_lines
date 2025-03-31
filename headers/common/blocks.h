#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <common/data_types.h>
#include <optional>

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

    virtual bool read(TaggedBlockReader *reader, BlockInfo &info);

    static void lookup(Block *&block, const BlockInfo &info);
};

struct AuthorIdsBlock final : public Block {
    std::map<uint32_t, std::string> authorIds;
    bool read(TaggedBlockReader *reader, BlockInfo &info) override;
};

struct MigrationInfoBlock final : public Block {
    CrdtId migrationId;
    bool isDevice;
    bool read(TaggedBlockReader *reader, BlockInfo &info) override;
};

struct PageInfoBlock final : public Block {
    uint32_t loadsCount;
    uint32_t mergesCount;
    uint32_t textCharsCount;
    uint32_t textLinesCount;
    uint32_t typeFolioUseCount = 0;
    bool read(TaggedBlockReader *reader, BlockInfo &info) override;
};

struct SceneInfoBlock final : public Block {
    LwwItem<CrdtId> currentLayer;
    std::optional<LwwItem<bool>> backgroundVisible;
    std::optional<LwwItem<bool>> rootDocumentVisible;
    std::optional<LwwItem<IntPair>> paperSize;
    bool read(TaggedBlockReader *reader, BlockInfo &info) override;
};

struct UnreadableBlock final : public Block {
};

#endif //BLOCKS_H
