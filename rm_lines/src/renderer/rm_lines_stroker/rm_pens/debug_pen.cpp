#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"

void DebugPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;
    for (int i = 0; i < length; ++i) {
        dst[i] = fill->baseColor.toRGBA();
        v = v + dx;
    }
}
