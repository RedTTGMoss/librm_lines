#include "renderer/text/font_manager.h"
#include "sans.h"
#include "sans_italic.h"
#include "serif.h"
#include "serif_italic.h"

FontManager &FontManager::instance() {
    static FontManager manager;
    return manager;
}

FontManager::FontManager() {
    FT_Init_FreeType(&library);

    FT_New_Memory_Face(
        library,
        sansFontData,
        sizeof(sansFontData),
        0,
        &m_sans
    );

    FT_New_Memory_Face(
        library,
        sansItalicFontData,
        sizeof(sansItalicFontData),
        0,
        &m_sansItalic
    );

    FT_New_Memory_Face(
        library,
        serifFontData,
        sizeof(serifFontData),
        0,
        &m_serif
    );

    FT_New_Memory_Face(
        library,
        serifItalicFontData,
        sizeof(serifItalicFontData),
        0,
        &m_serifItalic
    );
}

FontManager::~FontManager() {
    if (m_sans)
        FT_Done_Face(m_sans);

    if (m_sansItalic)
        FT_Done_Face(m_sansItalic);

    if (m_serif)
        FT_Done_Face(m_serif);

    if (m_serifItalic)
        FT_Done_Face(m_serifItalic);

    if (library)
        FT_Done_FreeType(library);
}

FT_Face FontManager::selectFont(const FontType font, const bool italic) {
    switch (font) {
        case Sans:
            return italic ? m_sansItalic : m_sans;
        case Serif:
            return italic ? m_serifItalic : m_serif;
        default:
            return m_sans;
    }
}
