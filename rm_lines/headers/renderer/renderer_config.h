#pragma once

enum BackdropAlign {
    BACKDROP_ALIGN_CENTER = 0,
    BACKDROP_ALIGN_TOP_LEFT = 1,
    BACKDROP_ALIGN_TOP_CENTER = 2,
    BACKDROP_ALIGN_TOP_RIGHT = 3,
    BACKDROP_ALIGN_BOTTOM_LEFT = 4,
    BACKDROP_ALIGN_BOTTOM_CENTER = 5,
    BACKDROP_ALIGN_BOTTOM_RIGHT = 6,
    BACKDROP_ALIGN_LEFT_CENTER = 7,
    BACKDROP_ALIGN_RIGHT_CENTER = 8
};

struct RendererConfig {
    const uint8_t configVersion = 4;
    int8_t penWhitelist[20] = {};
    int8_t penBlacklist[20] = {};
    bool useWhitelist = false;
    CrdtId disabledLayers[10] = {};
    bool enableText = true;
    bool enableImages = true;
    bool enableGlyphHighlights = true;
    bool enableBackdrop = true;
    bool useBackdropForSamplingOnly = false;
    bool followRulesInJson = false;
    bool strokerDataInJson = false;
    float backdropOffsetX = 0.0f;
    float backdropOffsetY = 0.0f;
    uint8_t backdropAlign = 2; // BackdropAlign TYPE

    RendererConfig() {
        std::ranges::fill(penWhitelist, -1);
        std::ranges::fill(penBlacklist, -1);
        std::ranges::fill(disabledLayers, END_MARKER);
    }
};
