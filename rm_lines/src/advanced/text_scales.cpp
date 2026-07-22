#include "advanced/text.h"
#include "advanced/text_scale.h"

/*
 *  This file handles all the raw values that are closely matched to fit the
 *  math and look behind the size of text, and the resulting anchors.
 *  The following data in this file has been sampled from a mix of:
 *  - on device data
 *  - PDF exports
 *  - side-by-side overlay comparisons and adjustments
 *  A lot of the data here is GUESS WORK.
 *  If you have any real data of the values used by remarkable, I'd love if you make a PR <3
 *  Until then, the goal in this file is to closely match the shapes, sizes and looks of
 *  the text rendered, when using the REMARKABLE FONTS, which will not be redistributed here,
 *  so if you find the look of text inconsistent, then consider switching which font is getting used
 *  and if using this for personal use, a download tool for the fonts is available and the build system
 *  will use it automatically. reMarkable fonts are licensed and copyrighted, use them at your own discretion
 */

constexpr float TEXT_TOP_Y = 139;
constexpr float TEXT_WIDTH_ALIGN = 0; // Deprecated in use of margins
constexpr float TAB_LENGTH = 48.105729420979856;

constexpr StyleScaleEntry EndOfStyleList = {END_STYLE_LIST, 0};
constexpr StyleNestedScaleEntry EndOfNestedStyleList = {END_STYLE_LIST, nullptr};

namespace {
    // Related to the column width
    constexpr float MEDIUM_WIDTH_PERCENT = 2.0f / 3.0f;
    constexpr float WIDE_WIDTH_PERCENT = 32.0f / 39.0f;
    constexpr float NARROW_WIDTH_PERCENT = 185.0f / 351.0f;
    constexpr float TEXT_Y_PERCENT = 1.0f / 8.0f;

    // Most values are the same, we can edit them here
    constexpr StyleScaleValue TITLE_LINE_HEIGHT = 64.14;
    constexpr StyleScaleValue SUB_LINE_HEIGHT = 36.08;
    constexpr StyleScaleValue BASIC_LINE_HEIGHT = 32.07;

    // Any extra gap between the bound start and the text
    constexpr StyleScaleList StyleLeftMargins = {
        {
            {BASIC, 52.62994702657062},
            {PlainText, 4.52421760559082},
            {Sub, 4.52421760559082},
            {Title, 4.52421760559082},
            EndOfStyleList
        }
    };


    // The font size
    constexpr StyleScaleList FontSizes = {
        {
            {BASIC, BASIC_LINE_HEIGHT},
            {PlainText, BASIC_LINE_HEIGHT},
            {Title, TITLE_LINE_HEIGHT},
            {Sub, SUB_LINE_HEIGHT},
            EndOfStyleList
        }
    };

    constexpr StyleScaleList StyleWeights = {
        {
            {BASIC, 400},
            {Sub, 500},
            EndOfStyleList
        }
    };

    constexpr StyleScaleList StyleWeightsItalic = {
        {
            {BASIC, 400},
            EndOfStyleList
        }
    };

    constexpr StyleScaleList StyleWeightsBold = {
        {
            {BASIC, 700},
            {Sub, 800},
            EndOfStyleList
        }
    };

    constexpr StyleScaleList Bullet_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList BulletTab_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList CheckBox_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList CheckBoxChecked_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList CheckBoxTab_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList CheckBoxTabChecked_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList Numbered_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList PlainText_Rel = {
        {
            {BASIC, 69.70004134707983},
            {Sub, 89.44656954871289},
            {Title, 163.515625},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList TextTop_Rel = {
        {
            {BASIC, 128.18315929836695},
            {Sub, 131.8944443596734},
            {Title, 157.85774628321332},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList Sub_Rel = {
        {
            {BASIC, 63.43628353542749},
            {Sub, 83.18281173706055},
            {Title, 157.2518189748128},
            EndOfStyleList
        }
    };
    constexpr StyleScaleList Title_Rel = {
        {
            {BASIC, 70.54565853542749},
            {Sub, 90.29218673706055},
            {Title, 164.36121808158026},
            EndOfStyleList
        }
    };

    constexpr NestedStyleScaleList StyleHeightsRelative = {
        {
            // This is the TOP style
            {TextTop, &TextTop_Rel},
            {Title, &Title_Rel},
            {Sub, &Sub_Rel},
            {PlainText, &PlainText_Rel},
            {Bullet, &Bullet_Rel},
            {BulletTab, &BulletTab_Rel},
            {Numbered, &Numbered_Rel},
            {NumberedTab, &Numbered_Rel},
            {CheckBox, &CheckBox_Rel},
            {CheckBoxTab, &CheckBoxTab_Rel},
            {CheckBoxChecked, &CheckBoxChecked_Rel},
            {CheckBoxTabChecked, &CheckBoxTabChecked_Rel},
            // EndOfNestedStyleList
        }
    };
}

// Helper function to get a match from an array, or get a default
StyleScaleValue findStyleValue(const StyleScaleList &list, const ParagraphStyle &key) {
    for (const auto &[k, value]: list) {
        if (k == key)
            return value;
        if (k == END_STYLE_LIST)
            break; // A quick exit for short style list definitions
    }
    return list.front().second;
}

StyleScaleValue findNestedStyleValue(const NestedStyleScaleList &list, const ParagraphStyle &keyA,
                                     const ParagraphStyle &keyB) {
    for (const auto &[k, value]: list) {
        if (k == keyA)
            return findStyleValue(*value, keyB);
        if (k == END_STYLE_LIST)
            break; // A quick exit for short style list definitions
    }
    return findStyleValue(*list.front().second, keyB);
}

// THIS FUNCTION IS OLD, DO NOT USE IT.
StyleScaleValue getStyleHeight(const ParagraphStyle style) {
    return findNestedStyleValue(StyleHeightsRelative, TextTop, style);
}

StyleScaleValue getStyleHeight(const ParagraphStyle prevStyle, const ParagraphStyle style) {
    return findNestedStyleValue(StyleHeightsRelative, prevStyle, style);
}

StyleScaleValue getStyleMargin(const ParagraphStyle style) {
    return findStyleValue(StyleLeftMargins, style);
}

StyleScaleValue getFontSize(const ParagraphStyle style) {
    return findStyleValue(FontSizes, style);
}

StyleScaleValue getStyleWeight(const ParagraphStyle style, const TextFormattingOptions formatting) {
    // The order of the weights is important, because of how reMarkable choose to handle the italic weights
    // Bold (including BoldItalic) is first
    // Italic is second
    // Regular is last
    return findStyleValue(
        formatting.bold
            ? StyleWeightsBold
            : formatting.italic
                  ? StyleWeightsItalic
                  : StyleWeights, style
    );
}

StyleScaleValue getWidthPercent(const TextColumnWidth columnWidth) {
    switch (columnWidth) {
        case ColumnMedium:
            return MEDIUM_WIDTH_PERCENT;
        case ColumnWide:
            return WIDE_WIDTH_PERCENT;
        case ColumnNarrow:
            return NARROW_WIDTH_PERCENT;
    }
    return 0.0f;
}

TextAreaInfo getTextAreaInfo(const TextColumnWidth columnWidth) {
    // Apparently for compatibility, remarkable keeps all this math based on
    // The remarkable 2, and it should all be converted to per paperSize, once getting handled by respective device
    const float widthPercent = getWidthPercent(columnWidth);
    const float width = BASE_PAPER_SIZE_X * widthPercent;
    return {
        -(width / 2.0f),
        BASE_PAPER_SIZE_Y * TEXT_Y_PERCENT,
        width
    };
}
