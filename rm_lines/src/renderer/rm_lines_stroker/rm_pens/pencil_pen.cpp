#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "stb/stb_perlin.h"

void PencilPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;

    Varying2D baseV = v - Varying2D(fill->position->x, fill->position->y);

    for (int i = 0; i < length; ++i) {
        const float rawNoise = stb_perlin_noise3(baseV.x,
                                                 baseV.y, 0.0f,
                                                 0.0f, 0.0f, 0.0f);
        // const float n = (rawNoise + 1.0f) * 0.5f;
        const float n = rawNoise;
        if (fill->intensity == 1.0f || n < std::pow(fill->intensity * 2, 2.3f))
            dst[i] = fill->baseColor.toRGBA();
        v = v + dx;
        baseV = baseV + dx;
    }
}
