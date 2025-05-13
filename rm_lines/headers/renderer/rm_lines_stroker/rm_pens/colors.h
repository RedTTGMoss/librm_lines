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

inline Color blendMultiply(Color base, Color blend, float blend_amount) {
    if (IS_LIKELY(base.alpha == 0)) {
        return blend;
    }
    // BGRA
    // Convert to normalized [0,1] values.
    float base_r = base.red / 255.0f;
    float base_g = base.green / 255.0f;
    float base_b = base.blue / 255.0f;
    float base_a = base.alpha / 255.0f;
    float blend_r = blend.red / 255.0f;
    float blend_g = blend.green / 255.0f;
    float blend_b = blend.blue / 255.0f;
    float blend_a = blend.alpha / 255.0f;

    // Multiply blend mode for color channels.
    float mul_r = base_r * blend_r;
    float mul_g = base_g * blend_g;
    float mul_b = base_b * blend_b;

    // Interpolate between the base and the multiplied color based on blend_amount.
    float final_r = base_r * (1.0f - blend_amount) + mul_r * blend_amount;
    float final_g = base_g * (1.0f - blend_amount) + mul_g * blend_amount;
    float final_b = base_b * (1.0f - blend_amount) + mul_b * blend_amount;

    // Compute output alpha using standard alpha compositing.
    float out_a = blend_a + base_a * (1.0f - blend_a);

    // Convert back to [0,255] integer values.
    return Color(
        static_cast<unsigned char>(final_b * 255.0f),
        static_cast<unsigned char>(final_g * 255.0f),
        static_cast<unsigned char>(final_r * 255.0f),
        static_cast<unsigned char>(out_a * 255.0f)
    );
}
