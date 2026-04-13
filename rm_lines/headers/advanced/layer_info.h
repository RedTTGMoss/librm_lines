#pragma once
#include "common/scene_items.h"

namespace LayerInfo {
    struct LineInfo {
        Line line;
        CrdtId groupId;
        CrdtId itemId;
        float offsetX;
        float offsetY;
    };

    struct ImageInfo {
        Image image;
        CrdtId groupId;
        CrdtId itemId;
        float offsetX;
        float offsetY;
    };
}
