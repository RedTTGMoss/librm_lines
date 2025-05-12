#pragma once
#include "renderer/rm_lines_stroker/rm_pens/rm_pen_fill.h"
#include "renderer/rm_lines_stroker/rm_pens/colors.h"

void BasicPen(rMPenFill *fill, int x, int y, int length, Varying2D v, Varying2D dx);

void HighlighterPen(rMPenFill *fill, int x, int y, int length, Varying2D v, Varying2D dx);
