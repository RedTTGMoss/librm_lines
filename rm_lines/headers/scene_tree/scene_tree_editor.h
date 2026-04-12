#pragma once
#include <stack>

#include "scene_tree.h"
#include "scene_tree_export.h"

class LineBuilder;
class TextBuilder;

// A class that expands on the scene tree with functions for building out the scene tree
class SceneTreeEditor final : public SceneTree {
public:
    CrdtId createLayer(const std::string &label = "");

    SceneInfoBlock createSceneInfo();

    Group createSceneTree(const CrdtId &id, const std::string &label = "");

    CrdtId addSceneTree(const Group &&node);

    CrdtId addItemNode(SceneItemVariant item);

    void init();

    void initText();

    void initImageInfoBlock();

    // Edit functions that will be exposed
    LineBuilder startLine();

    std::string addImageInfo(std::string filename, const std::string &uuid);

    std::string addImageInfo(const std::string &filename) {
        return addImageInfo(filename, generateUUID());
    }

    CrdtId addImage(std::string uuid, std::vector<AdvancedMath::Vector> vertices);

    friend class LineBuilder;
    TextBuilder *text;

private:
    CrdtId currentLayer = ROOT_TEXT_NODE;
    CrdtId ids = ROOT_TEXT_NODE;
    std::vector<CrdtId> _layers;
};

class LineBuilder final : public Line {
public:
    explicit LineBuilder(SceneTreeEditor *editor, PenTool tool = PENCIL_1, PenColor color = BLACK);

    LineBuilder &addPoint(float x, float y);

    LineBuilder &setRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    LineBuilder &setRGBA(const Color &color);

    LineBuilder &setPen(PenTool tool);

    CrdtId endLine();

private:
    static uint8_t angleTo255(const float angle) {
        const float v = angle * 255.0f / (2.0f * PI);
        int iv = static_cast<int>(std::lround(v)) % 256;
        if (iv < 0) iv += 256;
        return static_cast<uint8_t>(iv);
    }

    static float wrapAngle(float a) {
        while (a < 0.0f) a += 2.0f * PI;
        while (a >= 2.0f * PI) a -= 2.0f * PI;
        return a;
    }

    static uint8_t angle255FromVec(const float dx, const float dy) {
        if (dx == 0.0f && dy == 0.0f)
            return 0;

        float a = std::atan2(dy, dx);
        a = wrapAngle(a);
        return angleTo255(a);
    }

    void assignDirections() {
        if (points.empty()) return;
        if (points.size() == 1) {
            points[0].direction = 0;
            return;
        }

        points[0].direction = angle255FromVec(
            points[1].x - points[0].x,
            points[1].y - points[0].y
        );

        for (size_t i = 1; i + 1 < points.size(); ++i) {
            const float dx = points[i + 1].x - points[i - 1].x;
            const float dy = points[i + 1].y - points[i - 1].y;
            points[i].direction = angle255FromVec(dx, dy);
        }

        const size_t last = points.size() - 1;
        points[last].direction = angle255FromVec(
            points[last].x - points[last - 1].x,
            points[last].y - points[last - 1].y
        );
    }

    uint32_t calculateDirection(const Point &prev, float x2, float y2);

    CrdtId nodeId;
    SceneTreeEditor *editor;
    uint32_t pointSpeed = 2;
    uint32_t pointDirection = 4;
    uint32_t pointWidth = 16;
    uint32_t pointPressure = 255;
};

class TextBuilder {
public:
    explicit TextBuilder(Text *text);

private:
    Text *text;
};
