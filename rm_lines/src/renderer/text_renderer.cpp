#include "renderer/text_renderer.h"
#include "renderer/renderer.h"
#include "sans.h"
#include "sans_italic.h"
#include "serif.h"
#include "serif_italic.h"

void TextRenderer::newParagraph(const Paragraph *next, const float scaleY) {
    paragraph = next;
    fontType = paragraph->style.value.getFont();
    lineHeight = paragraph->style.value.lineHeight();
    styleHeight = paragraph->style.value.styleHeight();
    rasterHeight = std::lround(lineHeight * scaleY);
    posY += styleHeight * scaleY;
}

void TextRenderer::newText(const FormattedText *next) {
    formattedText = next;
    weight = getStyleWeight(paragraph->style.value.legacy, formattedText->formatting);
    font = selectFont(fontType, formattedText->formatting.italic);
}

void TextRenderer::getGlyphs(const std::string &text, std::vector<GlyphLayout> &glyphs, uint32_t previous) {
    // logDebug(std::format("rasterHeight: {}", rasterHeight));
    const float scale = stbtt_ScaleForPixelHeight(font, static_cast<float>(rasterHeight - 1));
    // TODO: Fix overflow on text column bounds and text size
    // logDebug(std::format("scale: {}", scale));

    float x = boundStart;
    float y = posY;

    for (const char &character: text) {
        GlyphLayout glyph;
        int advance, lsb, xEnd, yEnd, kernAdvance = 0;
        if (previous)
            kernAdvance = std::lround(stbtt_GetCodepointKernAdvance(font, previous, character) * scale);
        previous = character;

        stbtt_GetCodepointHMetrics(font, character, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(font, character, scale, scale, &glyph.xOffset, &glyph.yOffset, &xEnd,
                                    &yEnd);
        if (kernAdvance)
            logDebug(std::format("Kern advance: {}", kernAdvance));
        glyph.advance = std::lround(advance * scale) + kernAdvance;
        glyph.codepoint = character;

        const float end = x + glyph.advance;
        glyph.width = xEnd - glyph.xOffset;
        glyph.height = yEnd - glyph.yOffset;

        if (end >= boundEnd) {
            x = boundStart;
            y += glyph.height;
            previous = 0;
            // TODO: Line space
        }

        glyph.x = x + glyph.xOffset;
        glyph.y = y + glyph.yOffset;

        x += glyph.advance;

        glyphs.push_back(glyph);
    }
}

stbtt_fontinfo *TextRenderer::selectFont(const FontType font, const bool italic) {
    // logDebug("Selecting font: " + std::string(italic ? "Italic" : "Regular") + " " +
    //          (font == Sans ? "Sans" : font == Serif ? "Serif" : "Unknown"));
    switch (font) {
        case Sans:
            return italic ? &g_sansItalicFont : &g_sansFont;
        case Serif:
            return italic ? &g_serifItalicFont : &g_serifFont;
        default:
            return &g_sansFont;
    }
}

TextRenderer::TextRenderer(Renderer *renderer) : renderer(renderer) {
    textMargin = renderer->getTextMargin();
    initializeFonts();
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
        renderer->stroker.moveTo(0, posY);
        renderer->stroker.lineTo(renderer->stroker.raster.x1, posY);
        renderer->stroker.finish();


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

bool TextRenderer::initializeFonts() {
    return
            stbtt_InitFont(&g_sansFont,
                           sansFontData,
                           stbtt_GetFontOffsetForIndex(sansFontData, 0)) &&

            stbtt_InitFont(&g_sansItalicFont,
                           sansItalicFontData,
                           stbtt_GetFontOffsetForIndex(sansItalicFontData, 0)) &&

            stbtt_InitFont(&g_serifFont,
                           serifFontData,
                           stbtt_GetFontOffsetForIndex(serifFontData, 0)) &&

            stbtt_InitFont(&g_serifItalicFont,
                           serifItalicFontData,
                           stbtt_GetFontOffsetForIndex(serifItalicFontData, 0));
}
