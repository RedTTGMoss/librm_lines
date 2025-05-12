#include "renderer/rm_lines_stroker/rm_pens/rm_pen_fill.h"

void rMPenFill::operator()(const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    assert(line && point);
    unsigned int *dst = buffer.scanline(y) + x;
    for (int i = 0; i < length; ++i) {
        if (line->argbColor.has_value()) {
            const auto [a, r, g, b] = line->argbColor.value();
            dst[i] = a << 24 | r << 16 | g << 8 | b;
        } else {
            dst[i] = 0xff000000;
        }

        v = v + dx;
    }
}

void rMPenFill::newLine() {
    assert(line);
    pen.line = line;
}

void rMPenFill::newPoint() {
    assert(point);
}
