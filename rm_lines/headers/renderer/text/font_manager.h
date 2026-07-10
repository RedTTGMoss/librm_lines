#pragma once
#include <ft2build.h>

#include "common/data_types.h"

#include FT_FREETYPE_H

class FontManager {
public:
    static FontManager &instance();

    FT_Face selectFont(FontType font, bool italic);

private:
    FontManager();

    ~FontManager();

    FontManager(const FontManager &) = delete;

    FontManager &operator=(const FontManager &) = delete;

    FT_Library library{};

    FT_Face m_sans{};
    FT_Face m_sansItalic{};
    FT_Face m_serif{};
    FT_Face m_serifItalic{};
};
