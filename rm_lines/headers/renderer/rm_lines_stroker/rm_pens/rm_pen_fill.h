#pragma once
#include "common/scene_items.h"
#include "renderer/rm_lines_stroker/rm_lines_stroker.h"

class rMPenFill;
using Varying2D = RMLinesRenderer::Varying2D;
using ImageBuffer = RMLinesRenderer::ImageBuffer;

typedef void OperatorFunction(rMPenFill *, int x, int y, int length, Varying2D v, Varying2D dx);

static auto TestLine = Line();
static auto TestPoint = Point();

inline std::string getPenToolName(const PenTool tool) {
    switch (tool) {
        case BALLPOINT_1:
            return "Ballpoint 1";
        case BALLPOINT_2:
            return "Ballpoint 2";
        case CALLIGRAPHY:
            return "Calligraphy";
        case ERASER:
            return "Eraser";
        case ERASER_AREA:
            return "Eraser area";
        case FINELINER_1:
            return "Fineliner 1";
        case FINELINER_2:
            return "Fineliner 2";
        case HIGHLIGHTER_1:
            return "Highlighter 1";
        case HIGHLIGHTER_2:
            return "Highlighter 2";
        case MARKER_1:
            return "Marker 1";
        case MARKER_2:
            return "Marker 2";
        case MECHANICAL_PENCIL_1:
            return "Mechanical pencil 1";
        case MECHANICAL_PENCIL_2:
            return "Mechanical pencil 2";
        case PAINTBRUSH_1:
            return "Paintbrush 1";
        case PAINTBRUSH_2:
            return "Paintbrush 2";
        case PENCIL_1:
            return "Pencil 1";
        case PENCIL_2:
            return "Pencil 2";
        case SHADER:
            return "Shader";
        default:
            return "Unknown pen type";
    }
}

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
    float previousIntensity;
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

    void debugTool(float width = 7.0f);
};
