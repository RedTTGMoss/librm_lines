#include "renderer/text_renderer.h"
#include "renderer/renderer.h"
#include "sans.h"
#include "serif.h"
#include "serif_italic.h"

TextRenderer::TextRenderer(Renderer *renderer) : renderer(renderer) {
}

stbtt_fontinfo *TextRenderer::selectFont(const FontType font, const bool italic) {
    switch (font) {
        case Sans:
            return &g_sansFont;
        case Serif:
            return italic ? &g_serifItalicFont : &g_serifFont;
        default:
            return &g_sansFont;
    }
}

void TextRenderer::renderText(const AdvancedMath::Vector *position, float scale) {
    if (renderer->textDocument.paragraphs.empty()) {
        return; // Early exit for no text
    }
    float yOffset = TEXT_TOP_Y;
    for (const auto &paragraph: renderer->textDocument.paragraphs) {
        const FontType font_type = paragraph.style.value.getFont();
        const float line_height = paragraph.style.value.getLineHeight();
        for (const auto &formattedText: paragraph.contents) {
            float weight = getStyleWeight(paragraph.style.value.legacy, formattedText.formatting.bold);
            const bool italic = formattedText.formatting.italic;
            stbtt_fontinfo *font = selectFont(font_type, italic);
        }
        yOffset += line_height;
    }
}

bool TextRenderer::initializeFonts() {
    return
            stbtt_InitFont(&g_sansFont,
                           sansFontData,
                           stbtt_GetFontOffsetForIndex(sansFontData, 0)) &&

            stbtt_InitFont(&g_serifFont,
                           serifFontData,
                           stbtt_GetFontOffsetForIndex(serifFontData, 0)) &&

            stbtt_InitFont(&g_serifItalicFont,
                           serifItalicFontData,
                           stbtt_GetFontOffsetForIndex(serifItalicFontData, 0));
}
