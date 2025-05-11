#pragma once
namespace RMLinesRenderer {
    template<typename FillFunction>
    struct MonoRaster {
        using Varyings = typename FillFunction::Varyings;

        void operator()(Triangle t, Varyings a, Varyings b, Varyings c);

        FillFunction fill;

        // ********************
        // Internals
        //

        void iterate(float &y, float yMax, float left, float right, float leftInc, float rightInc);
    };


    template<typename SpansConsumer>
    void MonoRaster<SpansConsumer>::operator()(Triangle t, Varyings, Varyings, Varyings) {
        t.sort();

        const float y0Floored = std::floor(t.a.y);
        float y;

        if (t.a.y - y0Floored <= 0.5f) {
            y = y0Floored + 0.5f;
        } else {
            y = y0Floored + 1.5f;
        }

        if (t.a.y != t.b.y) {
            const float difYAB = t.b.y - t.a.y;
            const float difYAC = t.c.y - t.a.y;
            float difXL = (t.b.x - t.a.x) / difYAB;
            float difXR = (t.c.x - t.a.x) / difYAC;
            const float yOffset = y - t.a.y;
            float left = t.a.x + difXL * yOffset + 0.5f;
            float right = t.a.x + difXR * yOffset + 0.5f;

            if (difXR < difXL) {
                std::swap(left, right);
                std::swap(difXL, difXR);
            }
            iterate(y, t.b.y, left, right, difXL, difXR);
        }

        if (t.b.y != t.c.y) {
            const float difYAC = t.c.y - t.a.y;
            const float difYBC = t.c.y - t.b.y;
            float difXL = (t.c.x - t.b.x) / difYBC;
            float difXR = (t.c.x - t.a.x) / difYAC;
            float left = t.c.x - (t.c.y - y) * difXL + 0.5f;
            float right = t.c.x - (t.c.y - y) * difXR + 0.5f;
            if (left > right) {
                std::swap(left, right);
                std::swap(difXL, difXR);
            }
            iterate(y, t.c.y, left, right, difXL, difXR);
        }
    }

    template<typename SpanConsumer>
    void MonoRaster<SpanConsumer>::iterate(float &y, const float yMax, float left, float right, const float leftInc,
                                           const float rightInc) {
        while (y < yMax) {
            int l = static_cast<int>(left);
            const int r = static_cast<int>(right);
            if (int len = r - l; len > 0) {
                fill(l, static_cast<int>(y), len, Varyings(), Varyings());
            }
            y += 1.0f;
            left += leftInc;
            right += rightInc;
        }
    }
}
