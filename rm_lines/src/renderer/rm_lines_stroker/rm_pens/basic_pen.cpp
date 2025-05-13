#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"

void BasicPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;
    for (int i = 0; i < length; ++i) {
        if (fill->line->argbColor.has_value()) {
            const auto color = fill->line->argbColor.value();
            dst[i] = color.toRGBA();
        } else {
            dst[i] = rMPallet[fill->line->color].second.toRGBA();
        }

        v = v + dx;
    }
}
