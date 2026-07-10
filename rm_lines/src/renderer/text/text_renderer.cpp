#include "renderer/text/text_renderer.h"
#include "renderer/renderer.h"

void TextRenderer::newParagraph(const Paragraph *next, const float scaleY) {
    paragraph = next;
    fontType = paragraph->style.value.getFont();
    lineHeight = paragraph->style.value.lineHeight();
    styleHeight = paragraph->style.value.styleHeight();
    rasterHeight = lineHeight * scaleY;
    posY += styleHeight * scaleY;
}

void TextRenderer::newText(const FormattedText *next) {
    formattedText = next;
    weight = getStyleWeight(paragraph->style.value.legacy, formattedText->formatting);
    font = FontManager::instance().selectFont(fontType, formattedText->formatting.italic);
}

void TextRenderer::getGlyphs(const std::string &text, std::vector<GlyphLayout> &glyphs, uint32_t previous) {
    // logDebug(std::format("rasterHeight: {}", rasterHeight));
    FT_Set_Pixel_Sizes(font, 0, rasterHeight);
    // TODO: Fix overflow on text column bounds and text size

    float x = boundStart;
    float y = posY;

    for (const char &character: text) {
        GlyphLayout glyph{};

        FT_UInt glyphIndex = FT_Get_Char_Index(font, character);

        // Load metrics only
        if (FT_Load_Glyph(font, glyphIndex, FT_LOAD_DEFAULT))
            continue;

        FT_GlyphSlot slot = font->glyph;

        glyph.codepoint = character;

        glyph.advance = slot->advance.x >> 6;

        glyph.width = slot->metrics.width >> 6;
        glyph.height = slot->metrics.height >> 6;

        glyph.xOffset = slot->metrics.horiBearingX >> 6;
        glyph.yOffset = -(slot->metrics.horiBearingY >> 6);

        const float end = x + glyph.advance;

        if (end >= boundEnd) {
            x = boundStart;
            y += rasterHeight; // use lineHeight later
            previous = 0;
        }

        glyph.x = x + glyph.xOffset;
        glyph.y = y + glyph.yOffset;

        x += glyph.advance;

        glyphs.push_back(glyph);

        previous = character;
    }
}

TextRenderer::TextRenderer(Renderer *renderer) : renderer(renderer) {
    textMargin = renderer->getTextMargin();
    // Ensure font manager instance
    FontManager::instance();
}

void TextRenderer::renderText(const AdvancedMath::Vector *position, const Vector scale) {
    if (renderer->textDocument.paragraphs.empty()) {
        return; // Early exit for no text
    }

    boundStart = (position->x + textMargin) * scale.x;
    boundEnd = (position->x + renderer->paperSize.first - textMargin) * scale.x;
    posY = (position->y + TEXT_TOP_Y) * scale.y;

    renderer->stroker.raster.raster.fill.baseColor = Color(192, 52, 235, 255);
    renderer->stroker.raster.raster.fill.debugTool(3.0f);
    for (const auto &next: renderer->textDocument.paragraphs) {
        newParagraph(&next, scale.y);


        for (const auto &formattedText: paragraph->contents) {
            newText(&formattedText);

            std::vector<GlyphLayout> glyphs;
            getGlyphs(formattedText.text, glyphs, 0);
            for (const auto &glyph: glyphs) {
                // logDebug(std::format("Glyph: {} at ({}, {}) size {}x{} offset ({}, {}) advance {}",
                //                      glyph.codepoint, glyph.x, glyph.y, glyph.width, glyph.height,
                //                      glyph.xOffset, glyph.yOffset, glyph.advance));
                renderer->stroker.moveTo(glyph.x, glyph.y);
                renderer->stroker.lineTo(glyph.x + glyph.width, glyph.y);
                renderer->stroker.lineTo(glyph.x + glyph.width, glyph.y + glyph.height);
                renderer->stroker.lineTo(glyph.x, glyph.y + glyph.height);
                renderer->stroker.lineTo(glyph.x, glyph.y);
                renderer->stroker.finish();
            }
        }
    }
}
