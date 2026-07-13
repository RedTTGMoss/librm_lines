#include "renderer/text/text_renderer.h"

#include "hb-ft.h"
#include "advanced/text_scale.h"
#include "renderer/renderer.h"

void TextRenderer::newParagraph(const Paragraph *next, const Vector scale) {
    paragraph = next;
    fontType = paragraph->style.value.getFont();
    fontSize = paragraph->style.value.fontSize();
    styleHeight = paragraph->style.value.styleHeight();
    scaledStyleHeight = styleHeight * scale.y;
    scaledFontSize = fontSize * scale.y;
    posX = boundStart;
    posY += styleHeight * scale.y;
}

void TextRenderer::newText(const FormattedText *next) {
    currentFormattedText = next;
    weight = getStyleWeight(paragraph->style.value.legacy, currentFormattedText->formatting);
    font = FontManager::instance().selectFont(fontType, currentFormattedText->formatting.italic);
    font->setWeight(weight);
    font->setSize(scaledFontSize);

    hbFont = font->getHb();
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

    for (unsigned int i = 0; i < glyphCount; i++) {
        GlyphLayout glyph{};

        const FT_UInt glyphIndex = glyphInfo[i].codepoint;

        if (FT_Load_Glyph(font->face, glyphIndex, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP))
            continue;

        FT_GlyphSlot slot = font->face->glyph;

        glyph.codepoint = glyphIndex;

        glyph.width = FT_TO_F(slot->metrics.width);
        glyph.height = FT_TO_F(slot->metrics.height);

        glyph.xOffset =
                FT_TO_F(glyphPos[i].x_offset) +
                FT_TO_F(slot->metrics.horiBearingX);

        glyph.yOffset = -FT_TO_F(slot->metrics.horiBearingY);

        glyph.advance = FT_TO_F(glyphPos[i].x_advance);


        if (posX + glyph.advance >= boundEnd) {
            posX = boundStart;
            posY += scaledFontSize;
        }

        glyph.x = posX + FT_TO_F(glyphPos[i].x_offset);
        glyph.y = posY + glyph.yOffset;

        posX += glyph.advance;

        glyphs.push_back(glyph);
    }

    hb_buffer_destroy(buffer);
}

void TextRenderer::getAllPageGlyphs(std::vector<GlyphLayout> &glyphs) {
    static const Vector position{0, 0};
    static const Vector scale{1, 1};
    prepareBounds(&position, scale);
    for (const auto &next: renderer->textDocument.paragraphs) {
        newParagraph(&next, scale);
        for (const auto &formattedText: paragraph->contents) {
            newText(&formattedText);
            getGlyphs(formattedText.text, glyphs);
        }
    }
}

void TextRenderer::prepareBounds(const Vector *position, const Vector scale) {
    // Currently we only adjust here for the column and Y
    // In the future it might be a good idea to limit the frame bounds too
    boundStart = (position->x + textMargin) * scale.x;
    boundEnd = (position->x + renderer->paperSize.first - textMargin) * scale.x;
    posY = (position->y + TEXT_TOP_Y) * scale.y;
}

TextRenderer::TextRenderer() : TextRenderer(nullptr) {
}

TextRenderer::TextRenderer(Renderer *renderer) {
    setRenderer(renderer);
    // Ensure font manager instance
    FontManager::instance();
}

void TextRenderer::setRenderer(Renderer *newRenderer) {
    this->renderer = newRenderer;
    if (!renderer)
        return;
    textMargin = renderer->getTextMargin();
}

void TextRenderer::renderText(const Vector *position, const Vector scale) {
    if (!renderer || renderer->textDocument.paragraphs.empty()) {
        return; // Early exit for no text
    }

    prepareBounds(position, scale);

    renderer->stroker.raster.raster.fill.baseColor = Color(192, 52, 235, 255);
    renderer->stroker.raster.raster.fill.debugTool(2.0f);
    for (const auto &next: renderer->textDocument.paragraphs) {
        newParagraph(&next, scale);


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
