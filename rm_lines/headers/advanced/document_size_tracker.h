#pragma once
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
                                 const bool landscape): documentCenter(0, 0),
                                                        documentCap(0, 0, 0, 0),
                                                        track(Rect::fromSides(
                                                            0, landscape ? frameSize.x : frameSize.y, 0,
                                                            landscape ? frameSize.y : frameSize.x)),
                                                        frameSize(landscape
                                                                      ? Vector(frameSize.y, frameSize.x)
                                                                      : frameSize),
                                                        offset(0, 0), pageType(pageType), landscape(landscape) {
        if (pageType == DOCUMENT)
            this->offset.x = this->frameSize.x * 0.2;
        // logDebug(std::format("Initial size tracker {}x{} AKA {}->{}x{}->{}, landscape: {}", frameSize.x, frameSize.y,
        //                      track.getLeft(), track.getRight(), track.getTop(), track.getBottom(), landscape));
    }

    DocumentSizeTracker(const float frameWidth, const float frameHeight, const PageType pageType,
                        const bool landscape) : DocumentSizeTracker(
        Vector(frameWidth, frameHeight), pageType, landscape) {
    }

    DocumentSizeTracker(const IntPair frameSize, const PageType pageType, const bool landscape) : DocumentSizeTracker(
        Vector(frameSize.first, frameSize.second), pageType, landscape) {
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
            track.setBottom(y);
        }
        if (y < track.getTop()) {
            track.setTop(y);
        }
        return y;
    }

    [[nodiscard]] float getFrameWidth() const {
        switch (pageType) {
            case NOTEBOOK:
                return frameSize.x;
            case DOCUMENT:
                return frameSize.x * 1.4;
            default:
                return -1;
        }
    }

    [[nodiscard]] float getFrameHeight() const {
        // This really isn't used anywhere since the horizontal coordinates are more important
        switch (pageType) {
            case NOTEBOOK:
                return frameSize.y;
            case DOCUMENT:
                return frameSize.y * 1.4;
            default:
                return -1;
        }
    }

    [[nodiscard]] float getTop() const {
        return track.getTop() + offset.y;
    }

    [[nodiscard]] float getBottom() const {
        return track.getBottom() + offset.y;
    }

    [[nodiscard]] float getLeft() const {
        return track.getLeft() + offset.x;
    }

    [[nodiscard]] float getRight() const {
        return track.getRight() + offset.x;
    }

private:
    Vector documentCenter;
    Rect documentCap;
    Rect track;
    Vector frameSize;
    Vector offset;
    PageType pageType;
    bool landscape;
};
