#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"

void ShaderPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;
    unsigned int *lineDst = fill->lineBuffer.scanline(y) + x;

    for (int i = 0; i < length; ++i) {
        if (lineDst[i] != fill->lineCounter) {
            // Save guard drawing to the same spot during a single line draw
            // Shader/blend pen darkens like a transparent marker with low alpha (typically 64)
            dst[i] = blendShader(Color::fromARGB(&dst[i]), fill->baseColor).toARGB();
            lineDst[i] = fill->lineCounter;
        }

        v = v + dx;
    }
}
