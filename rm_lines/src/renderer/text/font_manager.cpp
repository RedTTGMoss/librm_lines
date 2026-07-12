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
    m_sans = new FontInfo();
    m_sansItalic = new FontInfo();
    m_serif = new FontInfo();
    m_serifItalic = new FontInfo();

    initFont(
        sansFontData,
        sizeof(sansFontData),
        m_sans
    );

    initFont(
        sansItalicFontData,
        sizeof(sansItalicFontData),
        m_sansItalic
    );

    initFont(
        serifFontData,
        sizeof(serifFontData),
        m_serif
    );

    initFont(
        serifItalicFontData,
        sizeof(serifItalicFontData),
        m_serifItalic
    );
}

FontManager::~FontManager() {
    if (m_sans)
        m_sans->cleanup();

    if (m_sansItalic)
        m_sansItalic->cleanup();

    if (m_serif)
        m_serif->cleanup();

    if (m_serifItalic)
        m_serifItalic->cleanup();

    delete m_sans;
    delete m_sansItalic;
    delete m_serif;
    delete m_serifItalic;

    if (library)
        FT_Done_FreeType(library);
}

void FontManager::initFont(const uint8_t *fontData, const size_t fontSize, FontInfo *info) {
    FT_New_Memory_Face(
        library,
        fontData,
        fontSize,
        0,
        &info->face
    );

    if (FT_Get_MM_Var(info->face, &info->mmVar) == 0) {
        for (FT_UInt i = 0; i < info->mmVar->num_axis; ++i) {
            switch (info->mmVar->axis[i].tag) {
                case FT_MAKE_TAG('w', 'g', 'h', 't'):
                    info->weightAxis = i;
                    break;

                case FT_MAKE_TAG('w', 'd', 't', 'h'):
                    info->widthAxis = i;
                    break;

                case FT_MAKE_TAG('s', 'l', 'n', 't'):
                    info->slantAxis = i;
                    break;

                case FT_MAKE_TAG('i', 't', 'a', 'l'):
                    info->italicAxis = i;
                    break;

                case FT_MAKE_TAG('o', 'p', 's', 'z'):
                    info->opticalSizeAxis = i;
                    break;
                default:
                    break;
            }
        }
    }
}

FontInfo *FontManager::selectFont(const FontType font, const bool italic) {
    switch (font) {
        case Sans:
            return italic ? m_sansItalic : m_sans;
        case Serif:
            return italic ? m_serifItalic : m_serif;
        default:
            return m_sans;
    }
}
