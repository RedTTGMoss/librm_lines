#pragma once
#include "library.h"

namespace RMLinesRenderer {
    struct [[nodiscard]] VaryingNoop {
        // ReSharper disable once CppNonExplicitConvertingConstructor
        VaryingNoop(float = 0.0, float = 0.0) { // NOLINT(*-explicit-constructor)
        }
    };


    struct [[nodiscard]] Varying2D {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct [[nodiscard]] Varying3D {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct [[nodiscard]] Varying4D {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;
    };

    struct VaryingGeneratorNoop {
        static VaryingNoop left(float, float);

        static VaryingNoop right(float, float);
    };

    struct VaryingGeneratorLengthWidth {
        Varying2D left(float strokeLength, float strokeWidth) const;

        Varying2D right(float strokeLength, float strokeWidth) const;

        float lengthFactor = 1.0f;
        float widthFactor = 1.0f;
    };


    inline Varying2D operator+(const Varying2D a, const Varying2D b) { return Varying2D(a.x + b.x, a.y + b.y); }
    inline Varying2D operator+(const Varying2D a, const float u) { return Varying2D(a.x + u, a.y + u); }
    inline Varying2D operator+(const float u, const Varying2D a) { return Varying2D(u + a.x, u + a.y); }
    inline Varying2D operator-(const Varying2D a, const Varying2D b) { return Varying2D(a.x - b.x, a.y - b.y); }
    inline Varying2D operator-(const Varying2D a, const float u) { return Varying2D(a.x - u, a.y - u); }
    inline Varying2D operator-(const float u, const Varying2D a) { return Varying2D(u - a.x, u - a.y); }
    inline Varying2D operator*(const Varying2D a, const float u) { return Varying2D(a.x * u, a.y * u); }
    inline Varying2D operator*(const float u, const Varying2D a) { return Varying2D(u * a.x, u * a.y); }

    inline Varying2D operator/(const Varying2D a, const float u) {
        if (IS_UNLIKELY(u == 0)) return Varying2D(0, 0);
        return Varying2D(a.x / u, a.y / u);
    }


    inline Varying3D operator+(const Varying3D a, const Varying3D b) {
        return Varying3D(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    inline Varying3D operator+(const Varying3D a, const float u) { return Varying3D(a.x + u, a.y + u, a.z + u); }
    inline Varying3D operator+(const float u, const Varying3D a) { return Varying3D(u + a.x, u + a.y, u + a.z); }

    inline Varying3D operator-(const Varying3D a, const Varying3D b) {
        return Varying3D(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    inline Varying3D operator-(const Varying3D a, const float u) { return Varying3D(a.x - u, a.y - u, a.z - u); }
    inline Varying3D operator-(const float u, const Varying3D a) { return Varying3D(u - a.x, u - a.y, u - a.z); }
    inline Varying3D operator*(const Varying3D a, const float u) { return Varying3D(a.x * u, a.y * u, a.z * u); }
    inline Varying3D operator*(const float u, const Varying3D a) { return Varying3D(u * a.x, u * a.y, u * a.z); }

    inline Varying3D operator/(const Varying3D a, const float u) {
        if (IS_UNLIKELY(u == 0)) return Varying3D(0, 0);
        return Varying3D(a.x / u, a.y / u, a.z / u);
    }


    inline Varying4D operator+(const Varying4D a, const Varying4D b) {
        return Varying4D(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }

    inline Varying4D operator+(const Varying4D a, const float u) {
        return Varying4D(a.x + u, a.y + u, a.z + u, a.w + u);
    }

    inline Varying4D operator+(const float u, const Varying4D a) {
        return Varying4D(u + a.x, u + a.y, u + a.z, u + a.w);
    }

    inline Varying4D operator-(const Varying4D a, const Varying4D b) {
        return Varying4D(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    }

    inline Varying4D operator-(const Varying4D a, const float u) {
        return Varying4D(a.x - u, a.y - u, a.z - u, a.w - u);
    }

    inline Varying4D operator-(const float u, const Varying4D a) {
        return Varying4D(u - a.x, u - a.y, u - a.z, u - a.w);
    }

    inline Varying4D operator*(const Varying4D a, const float u) {
        return Varying4D(a.x * u, a.y * u, a.z * u, a.w * u);
    }

    inline Varying4D operator*(const float u, const Varying4D a) {
        return Varying4D(u * a.x, u * a.y, u * a.z, u * a.w);
    }

    inline Varying4D operator/(const Varying4D a, const float u) {
        if (IS_UNLIKELY(u == 0)) return Varying4D(0, 0, 0, 0);
        return Varying4D(a.x / u, a.y / u, a.z / u, a.w / u);
    }


    inline VaryingNoop operator+(VaryingNoop, VaryingNoop) { return VaryingNoop(); }
    inline VaryingNoop operator+(VaryingNoop, float) { return VaryingNoop(); }
    inline VaryingNoop operator+(float, VaryingNoop) { return VaryingNoop(); }
    inline VaryingNoop operator-(VaryingNoop, VaryingNoop) { return VaryingNoop(); }
    inline VaryingNoop operator-(VaryingNoop, float) { return VaryingNoop(); }
    inline VaryingNoop operator-(float, VaryingNoop) { return VaryingNoop(); }
    inline VaryingNoop operator*(VaryingNoop, float) { return VaryingNoop(); }
    inline VaryingNoop operator*(float, VaryingNoop) { return VaryingNoop(); }
    inline VaryingNoop operator/(VaryingNoop, float) { return VaryingNoop(); }
}
