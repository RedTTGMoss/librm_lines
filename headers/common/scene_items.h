#ifndef LINE_H
#define LINE_H

#include <cstdint>
#include <vector>
#include <common/data_types.h>

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
};

struct Line {
    uint32_t toolId;
    // TODO: Maybe impl Pen type and add tool here
    uint32_t colorId;
    PenColor color;
    double thicknessScale;
    float startingLength;
    std::vector<Point> points;
    CrdtId timestamp;
    std::optional<CrdtId> moveId;

    bool read(TaggedBlockReader *reader, uint8_t version);
};



#endif //LINE_H
