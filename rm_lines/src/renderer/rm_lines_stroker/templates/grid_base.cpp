#include "renderer/rm_lines_stroker/rm_pens/pen_functions.h"
#include "renderer/rm_lines_stroker/templates/template_functions.h"

void GridBase(rMPenFill *fill, Renderer *renderer) {
    float cellSize;
    float width = 3.0f;
    float grid_offset_x = 0.0f;
    float grid_offset_y = 0.0f;
    if (renderer->templateName.find("small") != std::string::npos) {
        cellSize = 52.0f;
        width = 2.0f;
    } else if (renderer->templateName.find("medium") != std::string::npos) {
        cellSize = 78.0f;
    } else {
        cellSize = 104.0f;
    }
    if (renderer->landscape) {
        grid_offset_x = cellSize / 2.0f;
    }
    constexpr auto lineColor = Color(200, 200, 200);
    const float offset_x = (std::fmod(fill->position->x, cellSize) + grid_offset_x) * fill->scale;
    const float offset_y = (std::fmod(fill->position->y, cellSize) + grid_offset_y) * fill->scale;
    cellSize *= fill->scale;
    float x = offset_x;
    float y = offset_y;

    const auto stroker = renderer->getStroker();
    fill->operatorFunction = TemplatePen;
    fill->baseColor = lineColor;
    stroker->capStyle = CapStyle::FlatCap;
    stroker->joinStyle = RMLinesRenderer::BevelJoin;
    stroker->width = width * fill->scale;

    fill->line = &TestLine;
    fill->point = &TestPoint;

    while (x < fill->buffer.width) {
        stroker->moveTo(x, 0);
        stroker->lineTo(x, fill->buffer.height);
        stroker->finish();
        x += cellSize;
    }
    x = offset_x;
    while (y < fill->buffer.height) {
        stroker->moveTo(0, y);
        stroker->lineTo(fill->buffer.width, y);
        stroker->finish();
        y += cellSize;
    }
}
