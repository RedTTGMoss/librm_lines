#pragma once
#include "common/scene_items.h"
#include "common/data_types.h"

constexpr std::pair<PenColor, Color> rMPallet[] = {
    {BLACK, Color(0, 0, 0, 255)},
    {GRAY, Color(125, 125, 125, 255)},
    {WHITE, Color(255, 255, 255, 255)},
    {YELLOW, Color(99, 255, 255, 255)},
    {GREEN, Color(0, 255, 0, 255)},
    {PINK, Color(147, 20, 255, 255)},
    {BLUE, Color(204, 98, 0, 255)},
    {RED, Color(7, 7, 217, 255)},
    {GRAY_OVERLAP, Color(125, 125, 125, 255)},
    {GREEN_2, Color(113, 218, 145, 255)},
    {CYAN, Color(232, 210, 116, 255)},
    {MAGENTA, Color(210, 127, 192, 255)},
    {YELLOW_2, Color(25, 231, 250, 255)}
};

inline Color blendADD(const Color color, const Color base, float alpha) {
    // Blend the color with the base color using additive blending, whist accounting for alpha
    // the base color here is most probably solid, the color is also solid
    // but we want to use alpha to add it to the base
    return {
        static_cast<uint8_t>(std::min(255, static_cast<int>(color.blue * alpha + base.blue))),
        static_cast<uint8_t>(std::min(255, static_cast<int>(color.green * alpha + base.green))),
        static_cast<uint8_t>(std::min(255, static_cast<int>(color.red * alpha + base.red))),
        static_cast<uint8_t>(std::min(255, static_cast<int>(color.alpha * alpha + base.alpha)))
    };
};
