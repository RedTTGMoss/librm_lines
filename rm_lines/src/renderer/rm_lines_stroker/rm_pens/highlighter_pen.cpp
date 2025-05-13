#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"

void HighlighterPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;
    Color color;
    if (fill->line->argbColor.has_value()) {
        color = fill->line->argbColor.value();
    } else {
        color = rMPallet[fill->line->color].second;
    }

    for (int i = 0; i < length; ++i) {
        dst[i] = blendMultiply(Color::fromRGBA(&dst[i]), color, 0.25f).toRGBA();
        v = v + dx;
    }
}
