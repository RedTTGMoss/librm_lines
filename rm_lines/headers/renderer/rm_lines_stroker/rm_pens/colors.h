#pragma once
#include "common/scene_items.h"
#include "common/data_types.h"

constexpr Color getColorFromPalette(const PenColor penColor) {
    switch (penColor) {
        case BLACK: return Color(0, 0, 0, 255);
        case GRAY: return Color(125, 125, 125, 255);
        case WHITE: return Color(255, 255, 255, 255);
        case YELLOW: return Color(255, 255, 99, 255);
        case GREEN: return Color(0, 255, 0, 255);
        case PINK: return Color(255, 20, 147, 255);
        case BLUE: return Color(0, 98, 204, 255);
        case RED: return Color(217, 7, 7, 255);
        case GRAY_OVERLAP: return Color(125, 125, 125, 255);
        case GREEN_2: return Color(145, 218, 113, 255);
        case CYAN: return Color(116, 210, 232, 255);
        case MAGENTA: return Color(192, 127, 210, 255);
        case YELLOW_2: return Color(250, 231, 25, 255);
        default: return Color(0, 0, 0, 255);
    }
}

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

inline Color blendShader(const Color base, const Color blend) {
    if (IS_LIKELY(base.alpha == 0)) {
        // Single stroke on transparent background: slightly darker than just base color
        Color result = blend;
        result.alpha = result.alpha * 1.4f; // Slightly increase alpha for visibility
        return result;
    }

    // Shader/blend pen: standard alpha compositing
    // The blend color's alpha (typically 64) creates the watery, gradual effect
    const float blendAlpha = blend.alpha / 255.0f;
    const float baseAlpha = base.alpha / 255.0f;
    // Standard over operator: result = blend over base
    // result_color = (blend_color * blend_alpha) + (base_color * base_alpha * (1 - blend_alpha))
    // result_alpha = blend_alpha + base_alpha * (1 - blend_alpha)

    const float invBlendAlpha = 1.0f - blendAlpha;
    const float resultAlpha = blendAlpha + baseAlpha * invBlendAlpha;

    Color result;
    if (resultAlpha > 0.0f) {
        // Premultiply and composite
        result.red = static_cast<uint8_t>((blend.red * blendAlpha + base.red * baseAlpha * invBlendAlpha) /
                                          resultAlpha);
        result.green = static_cast<uint8_t>((blend.green * blendAlpha + base.green * baseAlpha * invBlendAlpha) /
                                            resultAlpha);
        result.blue = static_cast<uint8_t>((blend.blue * blendAlpha + base.blue * baseAlpha * invBlendAlpha) /
                                           resultAlpha);
        result.alpha = static_cast<uint8_t>(resultAlpha * 255.0f);
    } else {
        result = base;
    }

    return result;
}

