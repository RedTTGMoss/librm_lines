#pragma once

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"

struct TextSequence : CrdtSequence<TextItem> {
    // A custom CrdtSequence specifically for TextItem
    void expandTextItems();

    void compactTextItems();

    [[nodiscard]] std::vector<CrdtId> getSortedTextIds() const;

    bool operator==(char c) const;

private:
    bool expanded = false;
};
