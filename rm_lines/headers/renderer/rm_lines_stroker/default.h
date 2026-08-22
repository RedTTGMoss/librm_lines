#pragma once
#include "stroker.h"
#include "raster/clipped.h"
#include "raster/lerp.h"

namespace RMLinesRenderer {
    typedef Stroker<ClippedRaster<LerpRaster<rMPenFill> >, VaryingGeneratorLengthWidth> DefaultStrokerType;
}
