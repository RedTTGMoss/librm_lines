#pragma once
#include "common/data_types.h"

extern const float TEXT_TOP_Y;
extern const float TEXT_WIDTH_ALIGN;

struct TextAreaInfo {
    // Old style text area using coordinates
    float x;
    float y;
    // New style text area using column width (y is assumed by device I guess??)
    float width;
};

float getStyleHeight(ParagraphStyle style);

float getFontSize(ParagraphStyle style);

float getStyleWeight(ParagraphStyle style, TextFormattingOptions formatting);

float getWidthPercent(TextColumnWidth columnWidth);

TextAreaInfo getTextAreaInfo(IntPair paperSize, TextColumnWidth columnWidth);
