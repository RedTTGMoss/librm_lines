#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "stb/stb_perlin.h"

void BallpointPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;
    Color color;
    if (fill->line->argbColor.has_value()) {
        color = fill->line->argbColor.value();
    } else {
        color = rMPallet[fill->line->color].second;
    }

    Varying2D baseV = v - Varying2D(fill->position->x, fill->position->y);

    for (int i = 0; i < length; ++i) {
        const float n = stb_perlin_fbm_noise3(v.x, v.y, 0, 2.0, 0.5, 6);
        if (fill->intensity == 1.0f || n < std::pow(fill->intensity * 2, 2.3f)) {
            color.alpha = 255 * n;
            dst[i] = color.toRGBA();
        }
        v = v + dx;
        baseV = baseV + dx;
    }
}
