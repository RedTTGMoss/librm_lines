#pragma once
#include "common/data_types.h"

typedef double StyleScaleValue;
typedef const std::pair<const ParagraphStyle, StyleScaleValue> StyleScaleEntry;
typedef std::array<StyleScaleEntry, PARAGRAPH_STYLES_COUNT> StyleScaleList;
typedef const std::pair<const ParagraphStyle, const StyleScaleList *> StyleNestedScaleEntry;
typedef std::array<StyleNestedScaleEntry, PARAGRAPH_STYLES_COUNT> NestedStyleScaleList;

extern const StyleScaleValue TEXT_TOP_Y;
extern const StyleScaleValue TEXT_WIDTH_ALIGN;
extern const StyleScaleValue TAB_LENGTH;

struct TextAreaInfo {
    // Old style text area using coordinates
    float x;
    float y;
    // New style text area using column width (y is assumed by device I guess??)
    float width;
};

StyleScaleValue getStyleHeight(ParagraphStyle style);

StyleScaleValue getStyleHeight(ParagraphStyle prevStyle, ParagraphStyle style);

StyleScaleValue getStyleMargin(ParagraphStyle style);

StyleScaleValue getFontSize(ParagraphStyle style);

StyleScaleValue getStyleWeight(ParagraphStyle style, TextFormattingOptions formatting);

StyleScaleValue getWidthPercent(TextColumnWidth columnWidth);

TextAreaInfo getTextAreaInfo(TextColumnWidth columnWidth);
