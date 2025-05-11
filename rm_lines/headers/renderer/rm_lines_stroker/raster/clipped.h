#pragma once
#include "advanced/math.h"
using Triangle = AdvancedMath::Triangle;

namespace RMLinesRenderer {
    template<typename Raster>
    struct ClippedRaster {
        using Varyings = typename Raster::Varyings;

        void operator()(Triangle t, Varyings a, Varyings b, Varyings c) = delete;

        void clipLeft(Triangle t, Varyings a, Varyings b, Varyings c) = delete;

        void clipRight(const Triangle &t, Varyings a, Varyings b, Varyings c) = delete;

        void clipTop(Triangle t, Varyings a, Varyings b, Varyings c) = delete;

        void clipBottom(Triangle t, Varyings a, Varyings b, Varyings c) = delete;

        void ySort(Triangle &t, Varyings &a, Varyings &b, Varyings &c) = delete;

        void xSort(Triangle &t, Varyings &a, Varyings &b, Varyings &c) = delete;

        Raster raster;

        float x0 = 0.0f;
        float x1 = 0.0f;
        float y0 = 0.0f;
        float y1 = 0.0f;
    };
}
