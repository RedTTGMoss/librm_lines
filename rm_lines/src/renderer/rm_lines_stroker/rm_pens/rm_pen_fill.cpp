#include "renderer/rm_lines_stroker/rm_pens/rm_pen_fill.h"

#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"

void rMPenFill::operator()(const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    assert(line && point && operatorFunction);
    operatorFunction(this, x, y, length, v, dx);
}

void rMPenFill::newLine() {
    assert(line);

    switch (line->tool) {
        // case BALLPOINT_1:
        // case BALLPOINT_2:
        //     break;
        // case CALLIGRAPHY:
        //     break;
        // case ERASER:
        //     break;
        // case ERASER_AREA:
        //     break;
        // case FINELINER_1:
        // case FINELINER_2:
        //     break;
        case HIGHLIGHTER_1:
        case HIGHLIGHTER_2:
            operatorFunction = HighlighterPen;
            stroker->capStyle = RMLinesRenderer::SquareCap;
            stroker->width = 30 * scale;
            break;
        // case MARKER_1:
        // case MARKER_2:
        //     break;
        // case MECHANICAL_PENCIL_1:
        // case MECHANICAL_PENCIL_2:
        //     break;
        // case PAINTBRUSH_1:
        // case PAINTBRUSH_2:
        //     break;
        case PENCIL_1:
        case PENCIL_2:
            operatorFunction = PencilPen;
            stroker->capStyle = RMLinesRenderer::SquareCap;
            stroker->width = 30 * scale;
            break;
        // case SHADER:
        //     break;
        default:
            operatorFunction = BasicPen;
            stroker->capStyle = RMLinesRenderer::RoundCap;
            stroker->width = 10 * scale;
            break;
    }
}

void rMPenFill::newPoint() {
    assert(point);
}
