#pragma once
#include "advanced/math.h"
#include "advanced/text.h"
#include "common/data_types.h"
#include "../rm_lines_stroker/raster/clipped.h"
#include "font_manager.h"

class Renderer;

struct GlyphLayout {
    uint32_t codepoint;

    float x;
    float y;

    float width;
    float height;

    int xOffset;
    int yOffset;

    float advance;
};

class TextRenderer {
public:
    explicit TextRenderer(Renderer *renderer);

    ~TextRenderer() = default;

    void renderText(const AdvancedMath::Vector *position, Vector scale);

    void newParagraph(const Paragraph *next, float scaleY);

    void newText(const FormattedText *next);

    void getGlyphs(const std::string &text, std::vector<GlyphLayout> &glyphs, uint32_t previous);

private:
    float textMargin;

    // Positioning
    float posX;
    float posY;
    float boundStart;
    float boundEnd;

    // Font data
    FT_Face font;
    float weight;
    float lineHeight;
    float styleHeight;
    float rasterHeight;

    // Temporary
    FontType fontType;
    const Paragraph *paragraph;
    const FormattedText *formattedText;


    Renderer *renderer;
};
