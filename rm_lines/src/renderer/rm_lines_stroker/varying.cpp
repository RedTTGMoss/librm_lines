#include "renderer/rm_lines_stroker/varying.h"
using namespace RMLinesRenderer;
#include "library.h"

VaryingNoop VaryingGeneratorNoop::left(float, float) {
    return VaryingNoop();
}

VaryingNoop VaryingGeneratorNoop::right(float, float) {
    return VaryingNoop();
}


Varying2D VaryingGeneratorLengthWidth::left(const float strokeLength, const float strokeWidth) const {
    return Varying2D{strokeLength * lengthFactor, strokeWidth * widthFactor};
}


Varying2D VaryingGeneratorLengthWidth::right(const float strokeLength, const float strokeWidth) const {
    return Varying2D{strokeLength * lengthFactor, -strokeWidth * widthFactor};
}


