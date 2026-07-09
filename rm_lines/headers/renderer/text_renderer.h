#pragma once
#include "advanced/math.h"
#include "advanced/text.h"
#include "common/data_types.h"
#include "rm_lines_stroker/raster/clipped.h"
#include "stb/stb_truetype.h"

class Renderer;

struct GlyphLayout {
    uint32_t codepoint;

    int x;
    int y;

    int width;
    int height;

    int xOffset;
    int yOffset;

    int advance;
};

class TextRenderer {
public:
    explicit TextRenderer(Renderer *renderer);

    ~TextRenderer() = default;

    void renderText(const AdvancedMath::Vector *position, Vector scale);

    void newParagraph(const Paragraph *next, float scaleY);

    void newText(const FormattedText *next);

    void getGlyphs(const std::string &text, std::vector<GlyphLayout> &glyphs, uint32_t previous);

    stbtt_fontinfo *selectFont(FontType font, bool italic);

private:
    float textMargin;

    // Positioning
    float posX;
    float posY;
    float boundStart;
    float boundEnd;

    // Font data
    stbtt_fontinfo *font;
    float weight;
    float lineHeight;
    float styleHeight;
    int rasterHeight;

    // Temporary
    FontType fontType;
    const Paragraph *paragraph;
    const FormattedText *formattedText;


    Renderer *renderer;
    stbtt_fontinfo g_sansFont;
    stbtt_fontinfo g_sansItalicFont;
    stbtt_fontinfo g_serifFont;
    stbtt_fontinfo g_serifItalicFont;

    bool initializeFonts();
};
