#ifndef ADVANCED_TEXT_H
#define ADVANCED_TEXT_H

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"
#include "common/scene_tree.h"

void expandTextItems(CrdtSequence<TextItem> &textSequence);

inline void expandTextItems(SceneTree &tree) {
    if (!tree.rootText.has_value())
        return;
    return expandTextItems(tree.rootText->items);
}

inline void expandTextItems(SceneTree *tree) {
    return expandTextItems(*tree);
}

#endif //ADVANCED_TEXT_H
