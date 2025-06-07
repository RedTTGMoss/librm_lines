#pragma once
#include "common/scene_items.h"
#include "renderer/rm_lines_stroker/rm_lines_stroker.h"

class rMPenFill;
using Varying2D = RMLinesRenderer::Varying2D;
using ImageBuffer = RMLinesRenderer::ImageBuffer;

typedef void OperatorFunction(rMPenFill *, int x, int y, int length, Varying2D v, Varying2D dx);

static auto TestLine = Line();
static auto TestPoint = Point();

class rMPenFill {
public:
    typedef Varying2D Varyings;
    ImageBuffer buffer;
    ImageBuffer lineBuffer;
    const Line *line;
    const Point *point;
    const Vector *position;
    float baseWidth;
    unsigned int segmentCounter;
    unsigned int lineCounter;
    unsigned int pointCounter;
    float scale;
    float intensity;
    Color baseColor;
    OperatorFunction *operatorFunction;
    RMLinesRenderer::Stroker<RMLinesRenderer::ClippedRaster<RMLinesRenderer::LerpRaster<rMPenFill> >,
        RMLinesRenderer::VaryingGeneratorLengthWidth> *stroker;


    rMPenFill() = default;

    ~rMPenFill() = default;

    void operator()(int x, int y, int length, Varying2D v, Varying2D dx);

    void newLine();

    void newPoint();

    void reset();

    void debugTool(float width = 10.0f);
};
