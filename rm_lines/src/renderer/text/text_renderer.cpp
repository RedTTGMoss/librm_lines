#include "renderer/text/text_renderer.h"

#include "hb-ft.h"
#include "advanced/text_scale.h"
#include "renderer/renderer.h"
#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"

void TextRenderer::newParagraph(const Paragraph *next, const Vector scale) {
    paragraph = next;

    fontType = paragraph->style.value.getFont();
    fontSize = paragraph->style.value.fontSize();

    styleHeight = paragraph->style.value.styleHeight(prevStyle);
    styleMargin = paragraph->style.value.styleMargin();

    scaledStyleHeight = styleHeight * scale.y;
    scaledStyleMargin = styleMargin * scale.x;
    scaledFontSize = fontSize * scale.y;

    startPosX = boundStart + scaledStyleMargin;
    posX = startPosX;
    posY += scaledStyleHeight;

    prevStyle = next->style.value.getStyle();
}

void TextRenderer::newText(const FormattedText *next) {
    currentFormattedText = next;
    weight = getStyleWeight(paragraph->style.value.legacy, currentFormattedText->formatting);
    font = FontManager::instance().selectFont(fontType, currentFormattedText->formatting.italic, weight);
    font->setSize(scaledFontSize);

    hbFont = font->getHb();
}

void TextRenderer::getGlyphs(
    const FormattedText &text,
    std::vector<GlyphLayout> &glyphs,
    std::unordered_map<CrdtId, TextRect> &textRects
) {
    hb_buffer_t *buffer = hb_buffer_create();

    hb_buffer_add_utf8(
        buffer,
        text.text.c_str(),
        text.text.length(),
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

    std::vector<size_t> byteToCharacter(text.text.size() + 1);
    std::unordered_map<size_t, CrdtId> indexToCharId;

    size_t characterIndex = 0;

    for (size_t i = 0; i < text.text.size();) {
        byteToCharacter[i] = characterIndex++;

        const unsigned char c = text.text[i];

        size_t length =
                c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;

        for (size_t j = 1; j < length && i + j < text.text.size(); ++j)
            byteToCharacter[i + j] = characterIndex - 1;

        i += length;
    }

    int glyphIndex = 0;
    for (auto &charId: text.characterIDs) {
        // Index the characterId positions
        indexToCharId[glyphIndex++] = charId;

        // Reset the text rect for this character ID to default values
        textRects[charId] = {
            std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0, 0,
            fontSize, scaledFontSize, posY
        };
    }

    for (unsigned int i = 0; i < glyphCount; i++) {
        GlyphLayout glyph{};

        glyph.glyphIndex = glyphInfo[i].codepoint;

        if (FT_Load_Glyph(font->face, glyph.glyphIndex, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP))
            continue;

        const FT_GlyphSlot slot = font->face->glyph;

        glyph.codepoint = glyph.glyphIndex;

        glyph.width = FT_TO_F(slot->metrics.width);
        glyph.height = FT_TO_F(slot->metrics.height);

        glyph.xOffset =
                FT_TO_F(glyphPos[i].x_offset) +
                FT_TO_F(slot->metrics.horiBearingX);

        glyph.yOffset = -FT_TO_F(slot->metrics.horiBearingY);

        glyph.advance = FT_TO_F(glyphPos[i].x_advance);


        if (posX + glyph.advance >= boundEnd) {
            posX = startPosX;
            posY += scaledFontSize;
        }

        glyph.x = posX + FT_TO_F(glyphPos[i].x_offset);
        glyph.y = posY + glyph.yOffset;

        posX += glyph.advance;

        const auto charIndex = byteToCharacter[glyphInfo[i].cluster];
        const auto charId = indexToCharId[charIndex];

        textRects[charId].x = std::min(textRects[charId].x, glyph.x);
        textRects[charId].y = std::min(textRects[charId].y, glyph.y);
        textRects[charId].width = std::max(textRects[charId].width, glyph.width);
        textRects[charId].height = std::max(textRects[charId].height, glyph.height);

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
            getGlyphs(formattedText, glyphs, tempTextRects);
        }
    }
}

void TextRenderer::prepareBounds(const Vector *position, const Vector scale) {
    // Currently we only adjust here for the column and Y
    // In the future it might be a good idea to limit the frame bounds too
    boundStart = (position->x + textMargin) * scale.x;
    boundEnd = (position->x + renderer->paperSize.first - textMargin) * scale.x;
    posY = (position->y + TEXT_TOP_Y) * scale.y;
    prevStyle = TextTop;
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
            getGlyphs(formattedText, glyphs, tempTextRects);
            for (const auto &glyph: glyphs) {
                renderGlyph(glyph, position, scale);
            }
        }
    }

    // Debug text rects
    // for (const auto &[charId, rect]: tempTextRects) {
    //     renderer->stroker.moveTo(rect.x, rect.y);
    //     renderer->stroker.lineTo(rect.x + rect.width, rect.y);
    //     renderer->stroker.lineTo(rect.x + rect.width, rect.y + rect.height);
    //     renderer->stroker.lineTo(rect.x, rect.y + rect.height);
    //     renderer->stroker.lineTo(rect.x, rect.y);
    // }
}

void TextRenderer::renderGlyphHighlights(const Vector *position, Vector scale, const GlyphRange &glyphRange) {
    // Get the highlighter color
    if (glyphRange.color == ARGB) {
        renderer->stroker.raster.raster.fill.baseColor = glyphRange.argbColor;
        renderer->stroker.raster.raster.fill.baseColor.alpha = 64; // Fix it to 25%
    } else
        renderer->stroker.raster.raster.fill.baseColor = getHighlighterColorFromPalette(glyphRange.color);

    // Draw the rects
    for (const auto &rect: glyphRange.rects) {
        // Align to the middle of the page
        const auto startX = (position->x + rect.x + renderer->frameSize.halfX()) * scale.x;
        const auto startY = (position->y + rect.y) * scale.y;
        const auto width = rect.w * scale.x;
        const auto height = rect.h * scale.y;

        for (float y = startY; y < startY + height; ++y) {
            for (float x = startX; x < startX + width; ++x) {
                const uint32_t bufX = std::floor(x);
                const uint32_t bufY = std::floor(y);

                if (bufX >= renderer->stroker.raster.raster.fill.buffer.width ||
                    bufY >= renderer->stroker.raster.raster.fill.buffer.height)
                    continue;

                HighlighterPen(
                    &renderer->stroker.raster.raster.fill,
                    bufX,
                    bufY,
                    1,
                    {0, 0},
                    {0, 0}
                );
            }
            // Increment line counter to avoid drawing over the same pixels
            renderer->stroker.raster.raster.fill.lineCounter++;
        }
    }
}

void TextRenderer::renderGlyph(const GlyphLayout &glyph, const Vector *position, Vector scale) {
    FT_Load_Glyph(
        font->face,
        glyph.glyphIndex,
        FT_LOAD_DEFAULT
    );

    FT_Render_Glyph(
        font->face->glyph,
        FT_RENDER_MODE_NORMAL
    );

    drawBitmap(glyph.x, glyph.y, font->face->glyph->bitmap);
}

void TextRenderer::drawBitmap(float x, float y, const FT_Bitmap &bitmap) {
    const auto buf = &renderer->stroker.raster.raster.fill.buffer;
    if (x >= buf->width || y >= buf->height || x + bitmap.width < 0 || y + bitmap.rows < 0)
        return;

    for (unsigned int row = 0; row < bitmap.rows; ++row) {
        for (unsigned int col = 0; col < bitmap.width; ++col) {
            uint8_t pixelValue = bitmap.buffer[row * bitmap.pitch + col];
            // pixelValue /= 2;
            if (pixelValue > 0) {
                Color color(0, 0, 0, pixelValue);
                const uint32_t bufX = std::floor(x + col);
                const uint32_t bufY = std::floor(y + row);

                if (bufX >= buf->width || bufY >= buf->height)
                    continue;

                buf->scanline(bufY)[bufX] = color.toRGBA();
            }
        }
    }
}
