#ifndef ADVANCED_TEXT_H
#define ADVANCED_TEXT_H

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"

struct TextSequence : CrdtSequence<TextItem> {
    void expandTextItems();

private:
    bool expanded = false;
};

#endif //ADVANCED_TEXT_H
