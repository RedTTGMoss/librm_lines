#ifndef ADVANCED_TEXT_H
#define ADVANCED_TEXT_H

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"

constexpr std::array<std::pair<ParagraphStyle, int>, 8> LineHeights = {
    {
        {BASIC, 100},
        {PLAIN, 71},
        {HEADING, 150},
        {BOLD, 70},
        {BULLET, 35},
        {BULLET2, 35},
        {CHECKBOX, 100},
        {CHECKBOX_CHECKED, 100}
    }
};

constexpr CrdtId ANCHOR_ID_START = {0, 281474976710654};
constexpr CrdtId ANCHOR_ID_END = {0, 281474976710655};

struct TextSequence : CrdtSequence<TextItem> {
    void expandTextItems();

private:
    bool expanded = false;
};

#endif //ADVANCED_TEXT_H
