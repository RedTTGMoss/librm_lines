#pragma once
#include "renderer/rm_lines_stroker/rm_pens/rm_pen_fill.h"
#include "renderer/rm_lines_stroker/rm_pens/colors.h"

#define MAGIC_PENCIL_SIZE (44.6f * 2.3f)
#define K 5

void BasicPen(rMPenFill *fill, int x, int y, int length, Varying2D v, Varying2D dx);

void PencilPen(rMPenFill *fill, int x, int y, int length, Varying2D v, Varying2D dx);

void BallpointPen(rMPenFill *fill, int x, int y, int length, Varying2D v, Varying2D dx);

void HighlighterPen(rMPenFill *fill, int x, int y, int length, Varying2D v, Varying2D dx);

void DebugPen(rMPenFill *fill, int x, int y, int length, Varying2D v, Varying2D dx);
