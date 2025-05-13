#pragma once
#include "common/scene_items.h"
#include "renderer/rm_lines_stroker/rm_lines_stroker.h"

class rMPenFill;
using Varying2D = RMLinesRenderer::Varying2D;
using ImageBuffer = RMLinesRenderer::ImageBuffer;

typedef void OperatorFunction(rMPenFill *, int x, int y, int length, Varying2D v, Varying2D dx);

class rMPenFill {
public:
    typedef Varying2D Varyings;
    ImageBuffer buffer;
    const Line *line;
    const Point *point;
    const Vector *position;
    float scale;
    OperatorFunction *operatorFunction;
    RMLinesRenderer::Stroker<RMLinesRenderer::ClippedRaster<RMLinesRenderer::LerpRaster<rMPenFill> >,
        RMLinesRenderer::VaryingGeneratorLengthWidth> *stroker;


    rMPenFill() = default;

    ~rMPenFill() = default;

    void operator()(int x, int y, int length, Varying2D v, Varying2D dx);

    void newLine();

    void newPoint();
};
