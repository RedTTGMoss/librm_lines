#ifndef TEXT_H
#define TEXT_H

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"

struct Text;
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

struct FormattedText {
    std::string text;

    // Formatting options on this text
    bool bold = false;
    bool italic = false;

private:
    std::vector<CrdtId> textSequenceItems;
};

struct Paragraph {
    // Representing a single line which can contain multiple texts
    std::vector<FormattedText> contents;
    CrdtId startId;
    LwwItem<ParagraphStyle> style;
};

struct TextDocument {
    Text &text;
    // TODO: finish and impl
};

#endif //TEXT_H
