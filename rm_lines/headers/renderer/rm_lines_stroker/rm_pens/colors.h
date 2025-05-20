#pragma once
#include "common/scene_items.h"
#include "common/data_types.h"

constexpr std::pair<PenColor, Color> rMPallet[] = {
    {BLACK, Color{0, 0, 0, 255}},
    {GRAY, Color{125, 125, 125, 255}},
    {WHITE, Color{255, 255, 255, 255}},
    {YELLOW, Color{255, 255, 99, 255}},
    {GREEN, Color{0, 255, 0, 255}},
    {PINK, Color{255, 20, 147, 255}},
    {BLUE, Color{0, 98, 204, 255}},
    {RED, Color{217, 7, 7, 255}},
    {GRAY_OVERLAP, Color{125, 125, 125, 255}},
    {GREEN_2, Color{145, 218, 113, 255}},
    {CYAN, Color{116, 210, 232, 255}},
    {MAGENTA, Color{192, 127, 210, 255}},
    {YELLOW_2, Color{250, 231, 25, 255}}
};

inline Color blendMultiply(const Color base, const Color blend, const float blend_amount) {
    if (IS_LIKELY(base.alpha == 0)) {
        return blend;
    }
    const Color mul = base * blend;

    // Interpolate between the base and the multiplied color based on blend_amount.
    Color final = base * (1.0f - blend_amount) + mul * blend_amount;
    // Compute output alpha using standard alpha compositing.
    final.alpha = blend.alpha + base.alpha * (1.0f - blend.alpha);

    // Convert back to [0,255] integer values.
    return final;
}
