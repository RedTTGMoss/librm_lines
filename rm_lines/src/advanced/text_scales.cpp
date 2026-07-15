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

constexpr float TEXT_TOP_Y = 140;
constexpr float TEXT_WIDTH_ALIGN = -10;
constexpr float TAB_LENGTH = 10.0; // TODO: Update the tab length to be more proper

constexpr StyleScaleEntry EndOfStyleList = {END_STYLE_LIST, 0};
constexpr StyleNestedScaleEntry EndOfNestedStyleList = {END_STYLE_LIST, nullptr};

namespace {
    // Related to the column width
    constexpr float MEDIUM_WIDTH_PERCENT = 2.0f / 3.0f;
    constexpr float WIDE_WIDTH_PERCENT = 32.0f / 39.0f;
    constexpr float NARROW_WIDTH_PERCENT = 185.0f / 351.0f;
    constexpr float TEXT_Y_PERCENT = 1.0f / 8.0f;

    // Most values are the same, we can edit them here
    constexpr StyleScaleValue TITLE_LINE_HEIGHT = 65;
    constexpr StyleScaleValue SUB_LINE_HEIGHT = 37;
    constexpr StyleScaleValue BASIC_LINE_HEIGHT = 30;

    // The gap between the top and the first paragraph style
    constexpr StyleScaleList StyleTopMargins = {
        {
            {BASIC, 100},
            {PlainText, 120},
            {Title, 158},
            {Sub, 132},
            {Bullet, 71},
            EndOfStyleList
        }
    };


    // Any extra gap between the bound start and the text
    constexpr StyleScaleList StyleLeftMargins = {
        {
            {BASIC, 0},
            {PlainText, 2},
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
            {PlainText, 500},
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

    constexpr StyleScaleList Rel_BASIC = {
        {
            // Shared heights from top style
            {BASIC, 100},
            EndOfStyleList
        }
    };

    constexpr StyleScaleList Rel_Title = {
        {
            {BASIC, 164},
            {Sub, 157},
            EndOfStyleList
        }
    };

    constexpr StyleScaleList Rel_Sub = {
        {
            {Sub, 83},
            {Title, 89},
            EndOfStyleList
        }
    };

    constexpr StyleScaleList Rel_PlainText = {
        {
            {Title, 70},
            EndOfStyleList
        }
    };

    constexpr NestedStyleScaleList StyleHeightsRelative = {
        {
            // This is the bottom style
            {BASIC, &Rel_BASIC},
            {Title, &Rel_Title},
            {Sub, &Rel_Sub},
            {PlainText, &Rel_PlainText},
            {Bullet, &Rel_PlainText},
            {BulletTab, &Rel_PlainText},
            {Numbered, &Rel_PlainText},
            {NumberedTab, &Rel_PlainText},
            {CheckBox, &Rel_PlainText},
            {CheckBoxTab, &Rel_PlainText},
            {CheckBoxChecked, &Rel_PlainText},
            {CheckBoxTabChecked, &Rel_PlainText},
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

StyleScaleValue getStyleHeight(const ParagraphStyle style) {
    return findStyleValue(StyleTopMargins, style);
}

StyleScaleValue getStyleHeight(const ParagraphStyle prevStyle, const ParagraphStyle style) {
    if (prevStyle == TextTop) // Old default assumption
        return getStyleHeight(style);
    return findNestedStyleValue(StyleHeightsRelative, style, prevStyle);
}

StyleScaleValue getStyleMargin(const ParagraphStyle style) {
    return findStyleValue(StyleLeftMargins, style);
}

StyleScaleValue getFontSize(const ParagraphStyle style) {
    return findStyleValue(FontSizes, style);
}

float getStyleWeight(const ParagraphStyle style, const TextFormattingOptions formatting) {
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

float getWidthPercent(const TextColumnWidth columnWidth) {
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
