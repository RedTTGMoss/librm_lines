#pragma once
#include "advanced/math.h"
#include "advanced/text.h"
#include "common/data_types.h"
#include "../rm_lines_stroker/raster/clipped.h"
#include "font_manager.h"
#include "hb.h"

class Renderer;

struct GlyphLayout {
    uint32_t codepoint;
    FT_UInt glyphIndex;

    float x;
    float y;

    float width;
    float height;

    float xOffset;
    float yOffset;

    float advance;
};

class TextRenderer {
public:
    TextRenderer();

    TextRenderer(Renderer *renderer);

    void setRenderer(Renderer *newRenderer);

    ~TextRenderer() = default;

    void renderText(const Vector *position, Vector scale);

    void newParagraph(const Paragraph *next, Vector scale);

    void newText(const FormattedText *next);

    void getGlyphs(const std::string &text, std::vector<GlyphLayout> &glyphs);

    void getAllPageGlyphs(std::vector<GlyphLayout> &glyphs);

private:
    float textMargin = 0;

    // Positioning
    float posX = 0;
    float startPosX = 0;
    float posY = 0;
    float boundStart = 0;
    float boundEnd = 0;
    ParagraphStyle prevStyle = TextTop;

    // Font data
    FontInfo *font = nullptr;
    hb_font_t *hbFont = nullptr;
    float weight = 0;
    float fontSize = 0;
    float styleHeight = 0;
    float styleMargin = 0;
    float scaledFontSize = 0;
    float scaledStyleHeight = 0;
    float scaledStyleMargin = 0;

    // Temporary
    FontType fontType = Serif;
    const Paragraph *paragraph = nullptr;
    const FormattedText *currentFormattedText = nullptr;

    // Helpers
    void prepareBounds(const Vector *position, Vector scale);

    // Rendering
    void renderGlyph(const GlyphLayout &glyph, const Vector *position, Vector scale);

    void drawBitmap(float x, float y, const FT_Bitmap &bitmap);

    Renderer *renderer = nullptr;
};
