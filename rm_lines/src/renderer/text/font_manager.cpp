#include "renderer/text/font_manager.h"
#include "sans_300.h"
#include "sans_400.h"
#include "sans_500.h"
#include "sans_700.h"
#include "sans_800.h"
#include "sans_italic_300.h"
#include "sans_italic_400.h"
#include "sans_italic_500.h"
#include "sans_italic_700.h"
#include "sans_italic_800.h"
#include "serif_400.h"
#include "serif_700.h"
#include "serif_italic_400.h"
#include "serif_italic_700.h"

FontManager &FontManager::instance() {
    static FontManager manager;
    return manager;
}

FontManager::FontManager() {
    FT_Init_FreeType(&library);

    m_sans.addFont(initFont(sans_300FontData, sizeof(sans_300FontData)), 300);
    m_sans.addFont(initFont(sans_400FontData, sizeof(sans_400FontData)), 400);
    m_sans.addFont(initFont(sans_500FontData, sizeof(sans_500FontData)), 500);
    m_sans.addFont(initFont(sans_700FontData, sizeof(sans_700FontData)), 700);
    m_sans.addFont(initFont(sans_800FontData, sizeof(sans_800FontData)), 800);

    m_sansItalic.addFont(initFont(sans_italic_300FontData, sizeof(sans_italic_300FontData)), 300);
    m_sansItalic.addFont(initFont(sans_italic_400FontData, sizeof(sans_italic_400FontData)), 400);
    m_sansItalic.addFont(initFont(sans_italic_500FontData, sizeof(sans_italic_500FontData)), 500);
    m_sansItalic.addFont(initFont(sans_italic_700FontData, sizeof(sans_italic_700FontData)), 700);
    m_sansItalic.addFont(initFont(sans_italic_800FontData, sizeof(sans_italic_800FontData)), 800);

    m_serif.addFont(initFont(serif_400FontData, sizeof(serif_400FontData)), 400);
    m_serif.addFont(initFont(serif_700FontData, sizeof(serif_700FontData)), 700);

    m_serifItalic.addFont(initFont(serif_italic_400FontData, sizeof(serif_italic_400FontData)), 400);
    m_serifItalic.addFont(initFont(serif_italic_700FontData, sizeof(serif_italic_700FontData)), 700);
}

FontManager::~FontManager() {
    if (library)
        FT_Done_FreeType(library);
}

FontInfo FontManager::initFont(const uint8_t *fontData, const size_t fontSize) const {
    FontInfo info;
    FT_New_Memory_Face(
        library,
        fontData,
        fontSize,
        0,
        &info.face
    );

    if (FT_Get_MM_Var(info.face, &info.mmVar) == 0) {
        for (FT_UInt i = 0; i < info.mmVar->num_axis; ++i) {
            switch (info.mmVar->axis[i].tag) {
                case FT_MAKE_TAG('w', 'g', 'h', 't'):
                    info.weightAxis = i;
                    break;

                case FT_MAKE_TAG('w', 'd', 't', 'h'):
                    info.widthAxis = i;
                    break;

                case FT_MAKE_TAG('s', 'l', 'n', 't'):
                    info.slantAxis = i;
                    break;

                case FT_MAKE_TAG('i', 't', 'a', 'l'):
                    info.italicAxis = i;
                    break;

                case FT_MAKE_TAG('o', 'p', 's', 'z'):
                    info.opticalSizeAxis = i;
                    break;
                default:
                    break;
            }
        }
    }
    return info;
}

FontFamily *FontManager::selectFamily(const FontType font, const bool italic) {
    switch (font) {
        case Sans:
            return italic ? &m_sansItalic : &m_sans;
        case Serif:
            return italic ? &m_serifItalic : &m_serif;
        default:
            return &m_sans;
    }
}

FontInfo *FontManager::selectFont(const FontType font, const bool italic, const uint16_t weight) {
    FontFamily *family = selectFamily(font, italic);
    return family->getWeight(weight);
}
