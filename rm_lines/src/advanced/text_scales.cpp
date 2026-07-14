#include "advanced/text.h"
#include "advanced/text_scale.h"

namespace {
    // Related to the column width
    constexpr float MEDIUM_WIDTH_PERCENT = 2.0f / 3.0f;
    constexpr float WIDE_WIDTH_PERCENT = 32.0f / 39.0f;
    constexpr float NARROW_WIDTH_PERCENT = 185.0f / 351.0f;
    constexpr float TEXT_Y_PERCENT = 1.0f / 8.0f;

    // Most values are the same, we can edit them here
    constexpr int TITLE_LINE_HEIGHT = 66;
    constexpr int SUB_LINE_HEIGHT = 38;
    constexpr int BASIC_LINE_HEIGHT = 34;

    // The gap between paragraphs of different styles
    constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleHeights = {
        {
            {BASIC, 100},
            {PlainText, 120},
            {Title, 150},
            {Sub, 88},
            {Bullet, 71},
            {BulletTab, 71},
            {CheckBox, 71},
            {CheckBoxChecked, 71},
            {CheckBoxTab, 71},
            {CheckBoxTabChecked, 71},
            {Numbered, 71},
            {NumberedTab, 71}
        }
    };

    // The font size
    constexpr std::array<std::pair<ParagraphStyle, int>, 12> FontSizes = {
        {
            {BASIC, BASIC_LINE_HEIGHT},
            {PlainText, BASIC_LINE_HEIGHT},
            {Title, TITLE_LINE_HEIGHT},
            {Sub, SUB_LINE_HEIGHT},
            {Bullet, BASIC_LINE_HEIGHT},
            {BulletTab, BASIC_LINE_HEIGHT},
            {CheckBox, BASIC_LINE_HEIGHT},
            {CheckBoxChecked, BASIC_LINE_HEIGHT},
            {CheckBoxTab, BASIC_LINE_HEIGHT},
            {CheckBoxTabChecked, BASIC_LINE_HEIGHT},
            {Numbered, BASIC_LINE_HEIGHT},
            {NumberedTab, BASIC_LINE_HEIGHT}
        }
    };

    constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleWeights = {
        {
            {BASIC, 400},
            {Sub, 500},
        }
    };

    constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleWeightsItalic = {
        {
            {BASIC, 400},
        }
    };

    constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleWeightsBold = {
        {
            {BASIC, 700},
            {Sub, 800},
        }
    };
}

float getStyleHeight(const ParagraphStyle style) {
    for (const auto &[key, value]: StyleHeights) {
        if (key == style)
            return static_cast<float>(value);
    }
    return static_cast<float>(StyleHeights[0].second);
}

float getFontSize(const ParagraphStyle style) {
    for (const auto &[key, value]: FontSizes) {
        if (key == style)
            return static_cast<float>(value);
    }
    return static_cast<float>(FontSizes[0].second);
}

float getStyleWeight(const ParagraphStyle style, const TextFormattingOptions formatting) {
    // The order of the weights is important, because of how reMarkable choose to handle the italic weights
    // Bold (including BoldItalic) is first
    // Italic is second
    // Regular is last
    for (const auto &[key, value]: formatting.bold
                                       ? StyleWeightsBold
                                       : formatting.italic
                                             ? StyleWeightsItalic
                                             : StyleWeights) {
        if (key == style)
            return static_cast<float>(value);
    }
    return static_cast<float>(StyleWeights[0].second);
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

TextAreaInfo getTextAreaInfo(const IntPair paperSize, const TextColumnWidth columnWidth) {
    const float widthPercent = getWidthPercent(columnWidth);
    const float width = paperSize.first * widthPercent;
    return {
        -(width / 2.0f),
        paperSize.second * TEXT_Y_PERCENT,
        width
    };
}
