#pragma once

#include "advanced/rect.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

enum PageType {
    NOTEBOOK, // Notebooks have a fixed max size
    DOCUMENT // PDF & EPUB have different dynamic scaling and size (ZOOM)
};

class DocumentSizeTracker {
public:
    explicit DocumentSizeTracker(const Vector frameSize, const PageType pageType): documentCenter(0, 0),
        documentCap(0, 0, 0, 0),
        track(Rect::fromSides(
            0, frameSize.y, 0, frameSize.x)), frameSize(frameSize),
        offset(0, 0), pageType(pageType) {
        switch (pageType) {
            case DOCUMENT:
                this->offset.x = this->frameSize.x * 0.2;
                break;
            default:
                break;
        }
    }

    DocumentSizeTracker(const float frameWidth, const float frameHeight, const PageType pageType) : DocumentSizeTracker(
        Vector(frameWidth, frameHeight), pageType) {
    }

    DocumentSizeTracker(const IntPair frameSize, const PageType pageType) : DocumentSizeTracker(
        Vector(frameSize.first, frameSize.second), pageType) {
    }

    ~DocumentSizeTracker() = default;

    float trackX(const float x) {
        const float alignedX = x + getFrameWidth() / 2;
        if (alignedX > track.getRight()) {
            track.setRight(alignedX);
        }
        if (alignedX < track.getLeft()) {
            track.setLeft(alignedX);
        }
        return x;
    }

    float trackY(const float y) {
        if (y > track.getBottom()) {
            track.setRight(y);
        }
        if (y < track.getTop()) {
            track.setLeft(y);
        }
        return y;
    }

    float getFrameWidth() const {
        switch (pageType) {
            case NOTEBOOK:
                return reverseFrameSize ? frameSize.y : frameSize.x;
            case DOCUMENT:
                return frameSize.x * 1.4;
            default:
                return -1;
        }
    }

    float getFrameHeight() const {
        // This really isn't used anywhere since the horizontal coordinates are more important
        switch (pageType) {
            case NOTEBOOK:
                return reverseFrameSize ? frameSize.x : frameSize.y;
            case DOCUMENT:
                return frameSize.y * 1.4;
            default:
                return -1;
        }
    }

    bool reverseFrameSize = false; // This is for landscape!!!
    // DOCUMENT page types do not have landscape, this variable becomes obsolete in those cases.

private:
    Vector documentCenter;
    Rect documentCap;
    Rect track;
    Vector frameSize;
    Vector offset;
    PageType pageType;
};
