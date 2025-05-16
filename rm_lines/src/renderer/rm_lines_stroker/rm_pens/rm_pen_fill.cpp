#include "renderer/rm_lines_stroker/rm_pens/rm_pen_fill.h"

#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"

void rMPenFill::operator()(const int x, const int y, const int length, Varying2D v, const Varying2D dx) {
    assert(line && point && operatorFunction);
    operatorFunction(this, x, y, length, v, dx);
}

void rMPenFill::newLine() {
    assert(line);

    baseWidth = line->thicknessScale / 10;
    stroker->varying.lengthFactor = 1.0f;
    stroker->varying.widthFactor = 1.0f;

    switch (line->tool) {
        case BALLPOINT_1:
        case BALLPOINT_2:
            operatorFunction = BallpointPen;
            stroker->capStyle = RMLinesRenderer::RoundCap;
            stroker->varying.lengthFactor = 0.01f;
            stroker->varying.widthFactor = 0.5f;
            break;
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
            stroker->capStyle = RMLinesRenderer::RoundCap;
            break;
        // case SHADER:
        //     break;
        default:
            operatorFunction = BasicPen;
            stroker->capStyle = RMLinesRenderer::RoundCap;
            stroker->width = 20 * baseWidth * scale;
            break;
    }
    segmentCounter = 0;
    lineCounter++;
    pointCounter = 0;
}

void rMPenFill::newPoint() {
    assert(point);

    switch (line->tool) {
        case BALLPOINT_1:
        case BALLPOINT_2: {
            intensity = 0.1 * -(static_cast<float>(point->speed) / 4 / 35) + 1.2 * point->pressure / 255 + 0.6;
            // cap between 0 and 1
            intensity = std::max(0.0f, std::min(1.0f, intensity));
            const float segmentWidth = (0.5f + static_cast<float>(point->pressure) / 100.0f + 1.0f * static_cast<float>(
                                            point->width) / 4 - 0.5f * (
                                            static_cast<float>(point->speed) / 4 / 50)) * 2.0f * 2.3f;
            stroker->width = segmentWidth / K * scale;
            break;
        }
        case PENCIL_1:
        case PENCIL_2: {
            /* TODO: Improve Pencil intensity between points
             *
             * rM might be using an intensity between points, or depending on segmentation
             * This is a guess, but it would explain weird gaps in this code vs rM version which seems to have fills for
             * these gaps. There is also some noticeable dither-like effect in the rM version (on some lines).
             *
             * These observations can be seen in the pencil test file.
            */
            intensity = 0.1 *
                        -(static_cast<double>(point->speed) / 4 / 35) + 1 * static_cast<double>(point->
                            pressure) / 255 - 0.1 * AdvancedMath::directionToTilt(point->direction);

            // cap between 0 and 1
            intensity = std::max(0.15f, std::min(1.0f, intensity)) - 0.1f;
            const float segmentWidth = 10.0f * ((0.8f * baseWidth + 0.5 * point->pressure / 255.0f) * (
                                                    static_cast<float>(point->width) / 3.0f) -
                                                0.1f * AdvancedMath::directionToTilt(point->direction) -
                                                0.6f * (static_cast<float>(point->speed) / 4) / 10);
            const float maxWidth = baseWidth * MAGIC_PENCIL_SIZE;
            stroker->width = std::max(3.0f, std::min(segmentWidth, maxWidth)) / K * scale;
            break;
        }
        default:
            break;
    }
    segmentCounter++;
    pointCounter++;
}

void rMPenFill::reset() {
    segmentCounter = 0;
    lineCounter = 0;
    pointCounter = 0;

    buffer.release();
    lineBuffer.release();
}
