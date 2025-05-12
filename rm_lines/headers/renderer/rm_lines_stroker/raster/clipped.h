#pragma once
#include "advanced/math.h"
using Triangle = AdvancedMath::Triangle;
using Vector = AdvancedMath::Vector;

namespace RMLinesRenderer {
    template<typename Raster>
    struct ClippedRaster {
        using Varyings = typename Raster::Varyings;

        void operator()(Triangle t, Varyings a, Varyings b, Varyings c);

        void clipLeft(Triangle t, Varyings a, Varyings b, Varyings c);

        void clipRight(const Triangle &t, Varyings a, Varyings b, Varyings c);

        void clipTop(Triangle t, Varyings a, Varyings b, Varyings c);

        void clipBottom(Triangle t, Varyings a, Varyings b, Varyings c);

        void ySort(Triangle &t, Varyings &a, Varyings &b, Varyings &c);

        void xSort(Triangle &t, Varyings &a, Varyings &b, Varyings &c);

        Raster raster;

        float x0 = 0.0f;
        float x1 = 0.0f;
        float y0 = 0.0f;
        float y1 = 0.0f;
    };
}
