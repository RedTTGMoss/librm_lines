#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "stb/stb_perlin.h"

void PencilPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;
    Color color;
    if (fill->line->argbColor.has_value()) {
        color = fill->line->argbColor.value();
    } else {
        color = rMPallet[fill->line->color].second;
    }

    auto intensity = 0.1 *
                     -(static_cast<double>(fill->point->speed) / 4 / 35) + 1 * static_cast<double>(fill->point->
                         pressure) / 255;

    // cap between 0 and 1
    intensity = std::max(0.0, std::min(1.0, intensity));

    Varying2D baseV = v - Varying2D(fill->position->x, fill->position->y);

    for (int i = 0; i < length; ++i) {
        float n = stb_perlin_turbulence_noise3(baseV.x,
                                               baseV.y, 0.0f,
                                               3, 2, 1);
        if (n < intensity)
            dst[i] = color.toRGBA();
        v = v + dx;
        baseV = baseV + dx;
    }
}
