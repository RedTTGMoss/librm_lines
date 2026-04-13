#pragma once

#include <cstdint>
#include <vector>
#include <advanced/text_sequence.h>
#include <common/data_types.h>
#include <common/crdt_sequence_item.h>
#include <nlohmann/json.hpp>


class TaggedBlockWriter;
using json = nlohmann::json;

class TaggedBlockReader;

enum PenTool {
    BALLPOINT_1 = 2,
    BALLPOINT_2 = 15,
    CALLIGRAPHY = 21,
    ERASER = 6,
    ERASER_AREA = 8,
    FINELINER_1 = 4,
    FINELINER_2 = 17,
    HIGHLIGHTER_1 = 5,
    HIGHLIGHTER_2 = 18,
    MARKER_1 = 3,
    MARKER_2 = 16,
    MECHANICAL_PENCIL_1 = 7,
    MECHANICAL_PENCIL_2 = 13,
    PAINTBRUSH_1 = 0,
    PAINTBRUSH_2 = 12,
    PENCIL_1 = 1,
    PENCIL_2 = 14,
    SHADER = 23
};

enum PenColor {
    BLACK = 0,
    GRAY = 1,
    WHITE = 2,

    YELLOW = 3,
    GREEN = 4,
    PINK = 5,

    BLUE = 6,
    RED = 7,

    GRAY_OVERLAP = 8,

    ARGB = 9,

    GREEN_2 = 10,
    CYAN = 11,
    MAGENTA = 12,

    YELLOW_2 = 13,
};

struct Point {
    float x = 0;
    float y = 0;
    uint32_t speed = 2;
    uint32_t direction = 4;
    uint32_t width = 12;
    uint32_t pressure = 255; // FULL pressure

    bool read(TaggedBlockReader *reader, uint8_t version);

    bool write(TaggedBlockWriter *writer, uint8_t version) const;

    json toJson() const;
};

struct Line {
    PenTool tool = PENCIL_1;
    PenColor color = BLACK;
    double thicknessScale = 1.0;
    float startingLength = 0.0;
    std::vector<Point> points;
    CrdtId timestamp = ROOT_NODE;
    std::optional<CrdtId> moveId;
    std::optional<Color> argbColor;
    uint8_t version = 2;


    bool read(TaggedBlockReader *reader, uint8_t version);

    bool write(TaggedBlockWriter *writer) const;

    json toJson() const;
};

struct Text {
    TextSequence items;
    std::vector<TextFormat> styles;
    std::unordered_map<CrdtId, LwwItem<ParagraphStyleNew> > styleMap;
    double posX;
    double posY;
    LwwItem<float> width;

    json toJson() const;
};

struct GlyphRange {
    std::optional<uint32_t> start;
    std::optional<uint32_t> length;

    PenColor color;
    Color argbColor;
    std::string text;
    std::vector<AdvancedMath::Rect> rects;
    CrdtId firstId = END_MARKER;
    CrdtId lastId = END_MARKER;
    bool includeLastId = false;

    bool read(TaggedBlockReader *reader);

    bool write(TaggedBlockWriter *writer) const;

    json toJson() const;
};

struct Image {
    LwwItem<std::string> imageRef;
    CrdtId boundsTimestamp;
    std::vector<float> vertices;
    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

    std::optional<CrdtId> moveId;

    json toJson() const;
};
