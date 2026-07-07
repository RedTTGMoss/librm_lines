#pragma once
#include "advanced/math.h"
#include "common/data_types.h"
#include "stb/stb_truetype.h"

class Renderer;


class TextRenderer {
public:
    explicit TextRenderer(Renderer *renderer);

    ~TextRenderer() = default;

    stbtt_fontinfo *selectFont(FontType font, bool italic);

    void renderText(const AdvancedMath::Vector *position, float scale);

private:
    Renderer *renderer;
    stbtt_fontinfo g_sansFont;
    stbtt_fontinfo g_serifFont;
    stbtt_fontinfo g_serifItalicFont;

    bool initializeFonts();
};
