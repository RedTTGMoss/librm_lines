#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "stb/stb_perlin.h"

void PencilPen(rMPenFill *fill, const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    unsigned int *dst = fill->buffer.scanline(y) + x;

    Varying2D baseV = v - Varying2D{fill->position->x, fill->position->y};

    for (int i = 0; i < length; ++i) {
        const float n = stb_perlin_noise3(baseV.x,
                                          baseV.y, 0.0f,
                                          0.0f, 0.0f, 0.0f);

        // Lerp intensity based on segment position
        // v.x represents position along the stroke (0 at start, increases along segment)
        // Normalize to 0-1 range based on the segment length
        const float t = std::min(1.0f, std::max(0.0f, v.x / std::max(1.0f, v.x + dx.x * (length - i - 1))));
        const float lerpedIntensity = fill->previousIntensity + (fill->intensity - fill->previousIntensity) * t;

        if (lerpedIntensity == 1.0f || n < std::pow(lerpedIntensity * 2.0f, 2.3f)) {
            Color color = fill->baseColor;

            // Only reduce alpha for very light intensity strokes (< 0.2)
            if (lerpedIntensity < 0.2f) {
                // Gradual falloff from intensity 0.2->0.0 maps to alpha 1.0->0.4
                const float normalizedIntensity = lerpedIntensity / 0.2f;
                const float alphaMultiplier = 0.4f + 0.6f * std::pow(normalizedIntensity, 1.5f);
                color.alpha = static_cast<uint8_t>(color.alpha * alphaMultiplier);
            }

            const uint32_t colorRGBA = color.toARGB();
            if (dst[i] < colorRGBA) {
                dst[i] = colorRGBA;
            }
        }
        v = v + dx;
        baseV = baseV + dx;
    }
}
