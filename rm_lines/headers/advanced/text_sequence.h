#ifndef TEXT_SEQUENCE_H
#define TEXT_SEQUENCE_H

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"

struct TextSequence : CrdtSequence<TextItem> {
    // A custom CrdtSequence specifically for TextItem
    void expandTextItems();

private:
    bool expanded = false;
};

#endif //TEXT_SEQUENCE_H
