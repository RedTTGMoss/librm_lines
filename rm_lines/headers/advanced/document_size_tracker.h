#pragma once
#include "renderer/text/anchor_typedef.h"
using namespace AdvancedMath;
#include "advanced/math.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

enum PageType {
    NOTEBOOK, // Notebooks have a fixed max size
    DOCUMENT // PDF & EPUB have different dynamic scaling and size (ZOOM)
};

class DocumentSizeTracker {
public:
    explicit DocumentSizeTracker(const Vector frameSize, const PageType pageType,
                                 const bool landscape) : documentCenter(0, 0),
                                                         documentCap(0, 0, 0, 0),
                                                         track(Rect::fromSides(
                                                             0, landscape ? frameSize.x : frameSize.y, 0,
                                                             landscape ? frameSize.y : frameSize.x)),
                                                         frameSize(landscape
                                                                       ? Vector(frameSize.y, frameSize.x)
                                                                       : frameSize),
                                                         offset(0, 0), pageType(pageType), landscape(landscape) {
        // logDebug(std::format("Initial size tracker {}x{} AKA {}->{}x{}->{}, landscape: {}", frameSize.x, frameSize.y,
        //                      track.getLeft(), track.getRight(), track.getTop(), track.getBottom(), landscape));
    }

    DocumentSizeTracker(const StyleScaleValue frameWidth, const StyleScaleValue frameHeight, const PageType pageType,
                        const bool landscape) : DocumentSizeTracker(
        Vector(frameWidth, frameHeight), pageType, landscape) {
    }

    DocumentSizeTracker(const IntPair frameSize, const PageType pageType, const bool landscape) : DocumentSizeTracker(
        Vector(frameSize.first, frameSize.second), pageType, landscape) {
    }

    ~DocumentSizeTracker() = default;

    StyleScaleValue trackX(const StyleScaleValue x) {
        const StyleScaleValue alignedX = x + getFrameWidth() / 2;
        if (alignedX > track.getRight()) {
            track.setRight(alignedX);
        }
        if (alignedX < track.getLeft()) {
            track.setLeft(alignedX);
        }
        return x;
    }

    StyleScaleValue trackY(const StyleScaleValue y) {
        if (y > track.getBottom()) {
            track.setBottom(y);
        }
        if (y < track.getTop()) {
            track.setTop(y);
        }
        return y;
    }

    [[nodiscard]] StyleScaleValue getFrameWidth() const {
        return frameSize.x;
    }

    [[nodiscard]] StyleScaleValue getFrameHeight() const {
        // This really isn't used anywhere since the horizontal coordinates are more important
        return frameSize.y;
    }

    [[nodiscard]] StyleScaleValue getTop() const {
        return track.getTop() + offset.y;
    }

    [[nodiscard]] StyleScaleValue getBottom() const {
        return track.getBottom() + offset.y;
    }

    [[nodiscard]] StyleScaleValue getLeft() const {
        return track.getLeft() + offset.x;
    }

    [[nodiscard]] StyleScaleValue getRight() const {
        return track.getRight() + offset.x;
    }

    json toJson() const;

private:
    Vector documentCenter;
    Rect documentCap;
    Rect track;
    Vector frameSize;
    Vector offset;
    PageType pageType;
    bool landscape;
};

inline json DocumentSizeTracker::toJson() const {
    return {
        {"l", getLeft()},
        {"r", getRight()},
        {"t", getTop()},
        {"b", getBottom()},
        {"fw", getFrameWidth()},
        {"fh", getFrameHeight()}
    };
}
