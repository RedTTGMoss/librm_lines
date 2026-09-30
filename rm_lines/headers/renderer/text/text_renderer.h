#pragma once
#include "advanced/math.h"
#include "advanced/text.h"
#include "common/data_types.h"
#include "../rm_lines_stroker/raster/clipped.h"
#include "font_manager.h"
#include "hb.h"
#include "numbering_counter.h"
#include "advanced/text_scale.h"

#define TEXT_BULLET "•"
#define TEXT_SUBBULLET "◦"
#define TEXT_SUBSUBBULLET "-"
#define TEXT_CHECKBOX "\ue904"
#define TEXT_CHECKBOX_CHECKED "\ue905"
#define TEXT_CHECKBOX_HALF_CHECKED "\ue906"

class Renderer;

struct GlyphLayout {
    uint32_t codepoint;
    FT_UInt glyphIndex;

    StyleScaleValue x;
    StyleScaleValue y;

    StyleScaleValue width;
    StyleScaleValue height;

    StyleScaleValue xOffset;
    StyleScaleValue yOffset;

    StyleScaleValue advance;
};

struct TextRect {
    StyleScaleValue x;
    StyleScaleValue y;
    StyleScaleValue width;
    StyleScaleValue height;
    StyleScaleValue fontSize;
    StyleScaleValue fontSizeScaled;
    StyleScaleValue baseLine;
};

class TextRenderer {
public:
    TextRenderer();

    TextRenderer(Renderer *renderer);

    void setRenderer(Renderer *newRenderer);

    ~TextRenderer() = default;

    void renderText(const Vector *position, Vector scale);

    void renderGlyphHighlights(const Vector *position, Vector scale, const GlyphRange &glyphRange);

    void newParagraph(const Paragraph *next, Vector scale);

    void newText(const FormattedText *next);

    void markerFont();

    void getMarkerGlyphs(ParagraphStyleNew para, std::vector<GlyphLayout> &glyphs, Vector scale);


    void getGlyphs(std::string text, std::vector<GlyphLayout> &glyphs,
                   std::optional<std::unordered_map<CrdtId, TextRect> *> textRects = std::nullopt,
                   std::optional<const std::vector<CrdtId> *> characterIDs = std::nullopt);

    void getGlyphs(const FormattedText &text, std::vector<GlyphLayout> &glyphs,
                   std::unordered_map<CrdtId, TextRect> &textRects) {
        getGlyphs(text.text, glyphs, &textRects, &text.characterIDs);
    }


    void getAllPageGlyphs(std::vector<GlyphLayout> &glyphs);

    void getAnchors();

private:
    NumberingCounter numberingCounter;

    float textMargin = 0;

    // Positioning
    StyleScaleValue posX = 0;
    StyleScaleValue startPosX = 0;
    StyleScaleValue posY = 0;
    StyleScaleValue boundStart = 0;
    StyleScaleValue boundEnd = 0;
    ParagraphStyle prevStyle = TextTop;

    // Font data
    FontInfo *font = nullptr;
    hb_font_t *hbFont = nullptr;
    StyleScaleValue weight = 0;
    StyleScaleValue fontSize = 0;
    StyleScaleValue styleHeight = 0;
    StyleScaleValue styleMargin = 0;
    StyleScaleValue scaledFontSize = 0;
    StyleScaleValue scaledStyleHeight = 0;
    StyleScaleValue scaledStyleMargin = 0;

    // Temporary
    FontType fontType = Serif;
    const Paragraph *paragraph = nullptr;
    const FormattedText *currentFormattedText = nullptr;

    std::unordered_map<CrdtId, TextRect> tempTextRects;

    // Helpers
    void prepareBounds(const Vector *position, Vector scale);

    // Rendering
    void renderGlyph(const GlyphLayout &glyph, const Vector *position, Vector scale);

    void drawBitmap(StyleScaleValue x, StyleScaleValue y, const FT_Bitmap &bitmap);

    Renderer *renderer = nullptr;
};
