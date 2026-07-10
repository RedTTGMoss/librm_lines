#pragma once
#include <ft2build.h>

#include "hb-ft.h"
#include "hb.h"
#include "common/data_types.h"
#include "freetype/ftmm.h"

#include FT_FREETYPE_H

#define FT_TO_F(x) (x / 64.0f)
#define F_TO_FT(x) (x * 64.0f)
#define F_TO_FT_FIXED(x) (static_cast<int>(x) << 16)

struct FontInfo {
    FT_Face face{};
    FontType type;
    bool italic;
    float size = 0;
    hb_font_t *hbFont = nullptr;

    FT_MM_Var *mmVar = nullptr;

    FT_UInt weightAxis = UINT_MAX;
    FT_UInt widthAxis = UINT_MAX;
    FT_UInt slantAxis = UINT_MAX;
    FT_UInt italicAxis = UINT_MAX;
    FT_UInt opticalSizeAxis = UINT_MAX;

    void setWeight(const float weight) const {
        if (mmVar && weightAxis != UINT_MAX) {
            FT_Fixed normalizedWeight = F_TO_FT_FIXED(weight);
            FT_Set_Var_Design_Coordinates(face, 1, &normalizedWeight);
        }
    }

    void setSize(const float size) {
        FT_Set_Char_Size(
            face,
            0,
            F_TO_FT(size),
            0,
            0
        );
        if (size != this->size) {
            if (hbFont) {
                hb_font_destroy(hbFont);
                hbFont = nullptr;
            }
            this->size = size;
        }
    }

    hb_font_t *getHb() {
        if (!hbFont) {
            hbFont = hb_ft_font_create_referenced(face);
            hb_ft_font_set_load_flags(
                hbFont,
                FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP
            );
        }
        return hbFont;
    }

    void cleanup() const {
        if (hbFont)
            hb_font_destroy(hbFont);
        if (face)
            FT_Done_Face(face);
    }
};

class FontManager {
public:
    static FontManager &instance();

    FontInfo *selectFont(FontType font, bool italic);

private:
    FontManager();

    ~FontManager();

    FontManager(const FontManager &) = delete;

    void initFont(const uint8_t *fontData, size_t fontSize, FontInfo *info);

    FontManager &operator=(const FontManager &) = delete;

    FT_Library library{};

    FontInfo *m_sans{};
    FontInfo *m_sansItalic{};
    FontInfo *m_serif{};
    FontInfo *m_serifItalic{};
};
