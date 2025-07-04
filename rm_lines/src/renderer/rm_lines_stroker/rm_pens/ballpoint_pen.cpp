#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "stb/stb_perlin.h"

void BallpointPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;

    Varying2D baseV = v - Varying2D{fill->position->x, fill->position->y};

    for (int i = 0; i < length; ++i) {
        const float rawNoise = stb_perlin_fbm_noise3(v.x, v.y, 0, 2.0, 0.5, 6);
        if (const float n = (rawNoise + 1.0f) * 0.5f; n < fill->intensity) {
            Color color(
                fill->baseColor.red,
                fill->baseColor.green,
                fill->baseColor.blue,
                static_cast<uint8_t>(255.0f * std::max(n, fill->intensity))
            );
            dst[i] = color.toRGBA();
        }
        v = v + dx;
        baseV = baseV + dx;
    }
}
