#include "renderer/text/text_renderer.h"

#include "hb-ft.h"
#include "advanced/text_scale.h"
#include "renderer/renderer.h"

void TextRenderer::newParagraph(const Paragraph *next, const float scaleY) {
    paragraph = next;
    fontType = paragraph->style.value.getFont();
    fontSize = paragraph->style.value.fontSize();
    styleHeight = paragraph->style.value.styleHeight();
    scaledFontSize = fontSize * scaleY;
    posY += styleHeight * scaleY;
}

void TextRenderer::newText(const FormattedText *next) {
    formattedText = next;
    weight = getStyleWeight(paragraph->style.value.legacy, formattedText->formatting);
    font = FontManager::instance().selectFont(fontType, formattedText->formatting.italic);
    hbFont = hb_ft_font_create_referenced(font);
}

void TextRenderer::getGlyphs(
    const std::string &text,
    std::vector<GlyphLayout> &glyphs
) {
    FT_Set_Pixel_Sizes(font, 0, scaledFontSize);

    hb_buffer_t *buffer = hb_buffer_create();

    hb_buffer_add_utf8(
        buffer,
        text.c_str(),
        -1,
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
    hb_glyph_info_t *glyphInfo =
            hb_buffer_get_glyph_infos(buffer, &glyphCount);

    hb_glyph_position_t *glyphPos =
            hb_buffer_get_glyph_positions(buffer, &glyphCount);


    float x = boundStart;
    float y = posY;


    for (unsigned int i = 0; i < glyphCount; i++) {
        GlyphLayout glyph{};

        FT_UInt glyphIndex = glyphInfo[i].codepoint;

        if (FT_Load_Glyph(font, glyphIndex, FT_LOAD_DEFAULT))
            continue;

        FT_GlyphSlot slot = font->glyph;

        glyph.codepoint = glyphIndex;

        glyph.width = slot->metrics.width >> 6;
        glyph.height = slot->metrics.height >> 6;

        glyph.xOffset =
                (glyphPos[i].x_offset >> 6) +
                (slot->metrics.horiBearingX >> 6);

        glyph.yOffset =
                -(slot->metrics.horiBearingY >> 6);

        glyph.advance =
                glyphPos[i].x_advance >> 6;


        if (x + glyph.advance >= boundEnd) {
            x = boundStart;
            y += scaledFontSize;
        }

        glyph.x = x + (glyphPos[i].x_offset >> 6);
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
    renderer->stroker.raster.raster.fill.debugTool(3.0f);
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
