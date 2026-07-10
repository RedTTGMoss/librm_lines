#include "renderer/text/text_renderer.h"

#include "hb-ft.h"
#include "advanced/text_scale.h"
#include "renderer/renderer.h"

void TextRenderer::newParagraph(const Paragraph *next, const float scaleY) {
    paragraph = next;
    fontType = paragraph->style.value.getFont();
    fontSize = paragraph->style.value.fontSize();
    styleHeight = paragraph->style.value.styleHeight();
    scaledStyleHeight = styleHeight * scaleY;
    scaledFontSize = fontSize * scaleY;
    posY += styleHeight * scaleY;
}

void TextRenderer::newText(const FormattedText *next) {
    formattedText = next;
    weight = getStyleWeight(paragraph->style.value.legacy, formattedText->formatting);
    font = FontManager::instance().selectFont(fontType, formattedText->formatting.italic);
    FT_Set_Char_Size(
        font,
        0,
        F_TO_FT(scaledFontSize),
        0,
        0
    );
    if (hbFont)
        hb_font_destroy(hbFont);
    hbFont = hb_ft_font_create_referenced(font);
    hb_ft_font_set_load_flags(
        hbFont,
        FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP
    );
}

void TextRenderer::getGlyphs(
    const std::string &text,
    std::vector<GlyphLayout> &glyphs
) {
    hb_buffer_t *buffer = hb_buffer_create();

    hb_buffer_add_utf8(
        buffer,
        text.c_str(),
        text.length(),
        0,
        -1
    );

    hb_buffer_guess_segment_properties(buffer);

    hb_shape(
        hbFont,
        buffer,
        nullptr,
        0
    );

    unsigned int glyphCount;
    const hb_glyph_info_t *glyphInfo =
            hb_buffer_get_glyph_infos(buffer, &glyphCount);

    const hb_glyph_position_t *glyphPos =
            hb_buffer_get_glyph_positions(buffer, &glyphCount);


    float x = boundStart;
    float y = posY;


    for (unsigned int i = 0; i < glyphCount; i++) {
        GlyphLayout glyph{};

        const FT_UInt glyphIndex = glyphInfo[i].codepoint;

        if (FT_Load_Glyph(font, glyphIndex, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP))
            continue;

        FT_GlyphSlot slot = font->glyph;

        glyph.codepoint = glyphIndex;

        glyph.width = FT_TO_F(slot->metrics.width);
        glyph.height = FT_TO_F(slot->metrics.height);

        glyph.xOffset =
                FT_TO_F(glyphPos[i].x_offset) +
                FT_TO_F(slot->metrics.horiBearingX);

        glyph.yOffset = -FT_TO_F(slot->metrics.horiBearingY);

        glyph.advance = FT_TO_F(glyphPos[i].x_advance);


        if (x + glyph.advance >= boundEnd) {
            x = boundStart;
            y += scaledStyleHeight;
        }

        glyph.x = x + FT_TO_F(glyphPos[i].x_offset);
        glyph.y = y + glyph.yOffset;

        x += glyph.advance;

        glyphs.push_back(glyph);
    }

    hb_buffer_destroy(buffer);
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
    renderer->stroker.raster.raster.fill.debugTool(2.0f);
    for (const auto &next: renderer->textDocument.paragraphs) {
        newParagraph(&next, scale.y);


        for (const auto &formattedText: paragraph->contents) {
            newText(&formattedText);

            std::vector<GlyphLayout> glyphs;
            getGlyphs(formattedText.text, glyphs);
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
