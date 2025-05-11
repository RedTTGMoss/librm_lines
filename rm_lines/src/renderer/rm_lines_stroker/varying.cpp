#include "renderer/rm_lines_stroker/varying.h"
using namespace RMLinesRenderer;
#include "library.h"


Varying2D operator+(const Varying2D a, const Varying2D b) { return Varying2D(a.x + b.x, a.y + b.y); }
Varying2D operator+(const Varying2D a, const float u) { return Varying2D(a.x + u, a.y + u); }
Varying2D operator+(const float u, const Varying2D a) { return Varying2D(u + a.x, u + a.y); }
Varying2D operator-(const Varying2D a, const Varying2D b) { return Varying2D(a.x - b.x, a.y - b.y); }
Varying2D operator-(const Varying2D a, const float u) { return Varying2D(a.x - u, a.y - u); }
Varying2D operator-(const float u, const Varying2D a) { return Varying2D(u - a.x, u - a.y); }
Varying2D operator*(const Varying2D a, const float u) { return Varying2D(a.x * u, a.y * u); }
Varying2D operator*(const float u, const Varying2D a) { return Varying2D(u * a.x, u * a.y); }

Varying2D operator/(const Varying2D a, const float u) {
    if (IS_UNLIKELY(u == 0)) return Varying2D(0, 0);
    return Varying2D(a.x / u, a.y / u);
}


Varying3D operator+(const Varying3D a, const Varying3D b) { return Varying3D(a.x + b.x, a.y + b.y, a.z + b.z); }
Varying3D operator+(const Varying3D a, const float u) { return Varying3D(a.x + u, a.y + u, a.z + u); }
Varying3D operator+(const float u, const Varying3D a) { return Varying3D(u + a.x, u + a.y, u + a.z); }
Varying3D operator-(const Varying3D a, const Varying3D b) { return Varying3D(a.x - b.x, a.y - b.y, a.z - b.z); }
Varying3D operator-(const Varying3D a, const float u) { return Varying3D(a.x - u, a.y - u, a.z - u); }
Varying3D operator-(const float u, const Varying3D a) { return Varying3D(u - a.x, u - a.y, u - a.z); }
Varying3D operator*(const Varying3D a, const float u) { return Varying3D(a.x * u, a.y * u, a.z * u); }
Varying3D operator*(const float u, const Varying3D a) { return Varying3D(u * a.x, u * a.y, u * a.z); }

Varying3D operator/(const Varying3D a, const float u) {
    if (IS_UNLIKELY(u == 0)) return Varying3D(0, 0);
    return Varying3D(a.x / u, a.y / u, a.z / u);
}


Varying4D operator+(const Varying4D a, const Varying4D b) {
    return Varying4D(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

Varying4D operator+(const Varying4D a, const float u) { return Varying4D(a.x + u, a.y + u, a.z + u, a.w + u); }
Varying4D operator+(const float u, const Varying4D a) { return Varying4D(u + a.x, u + a.y, u + a.z, u + a.w); }

Varying4D operator-(const Varying4D a, const Varying4D b) {
    return Varying4D(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

Varying4D operator-(const Varying4D a, const float u) { return Varying4D(a.x - u, a.y - u, a.z - u, a.w - u); }
Varying4D operator-(const float u, const Varying4D a) { return Varying4D(u - a.x, u - a.y, u - a.z, u - a.w); }
Varying4D operator*(const Varying4D a, const float u) { return Varying4D(a.x * u, a.y * u, a.z * u, a.w * u); }
Varying4D operator*(const float u, const Varying4D a) { return Varying4D(u * a.x, u * a.y, u * a.z, u * a.w); }

Varying4D operator/(const Varying4D a, const float u) {
    if (IS_UNLIKELY(u == 0)) return Varying4D(0, 0, 0, 0);
    return Varying4D(a.x / u, a.y / u, a.z / u, a.w / u);
}


VaryingNoop operator+(VaryingNoop, VaryingNoop) { return VaryingNoop(); }
VaryingNoop operator+(VaryingNoop, float) { return VaryingNoop(); }
VaryingNoop operator+(float, VaryingNoop) { return VaryingNoop(); }
VaryingNoop operator-(VaryingNoop, VaryingNoop) { return VaryingNoop(); }
VaryingNoop operator-(VaryingNoop, float) { return VaryingNoop(); }
VaryingNoop operator-(float, VaryingNoop) { return VaryingNoop(); }
VaryingNoop operator*(VaryingNoop, float) { return VaryingNoop(); }
VaryingNoop operator*(float, VaryingNoop) { return VaryingNoop(); }
VaryingNoop operator/(VaryingNoop, float) { return VaryingNoop(); }

VaryingNoop VaryingGeneratorNoop::left(float, float) {
    return VaryingNoop();
}

VaryingNoop VaryingGeneratorNoop::right(float, float) {
    return VaryingNoop();
}


Varying2D VaryingGeneratorLengthWidth::left(const float strokeLength, const float strokeWidth) const {
    return Varying2D(strokeLength * lengthFactor, strokeWidth * widthFactor);
}


Varying2D VaryingGeneratorLengthWidth::right(const float strokeLength, const float strokeWidth) const {
    return Varying2D(strokeLength * lengthFactor, -strokeWidth * widthFactor);
}


