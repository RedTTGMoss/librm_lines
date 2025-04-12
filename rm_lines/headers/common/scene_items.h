#ifndef LINE_H
#define LINE_H

#include <cstdint>
#include <vector>
#include <common/data_types.h>
#include <common/crdt_sequence_item.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class TaggedBlockReader;

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


    // All highlight colors share the same value.
    // There is also yet unknown extra data in the block
    // that might contain additional color information.
    HIGHLIGHT = 9,

    GREEN_2 = 10,
    CYAN = 11,
    MAGENTA = 12,

    YELLOW_2 = 13,
};

struct Point {
    float x;
    float y;
    uint32_t speed;
    uint32_t direction;
    uint32_t width;
    uint32_t pressure;

    bool read(TaggedBlockReader *reader, uint8_t version);

    json toJson() const;
};

struct Line {
    uint32_t toolId;
    // TODO: Maybe impl Pen type and add tool here
    PenColor color;
    double thicknessScale;
    float startingLength;
    std::vector<Point> points;
    CrdtId timestamp;
    std::optional<CrdtId> moveId;
    std::optional<Color> argbColor;


    bool read(TaggedBlockReader *reader, uint8_t version);

    json toJson() const;
};

struct Text {
    CrdtSequence<TextItem> items;
    std::vector<TextFormat> styles;
    double posX;
    double posY;
    float width;

    json toJson() const;
};

struct GlyphRange {
    std::optional<uint32_t> start;
    std::optional<uint32_t> length;

    PenColor color;
    Color argbColor;
    std::string text;
    std::vector<Rect> rects;

    bool read(TaggedBlockReader *reader);

    json toJson() const;
};


#endif //LINE_H
