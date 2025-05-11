#include "renderer/rm_lines_stroker/rm_lines_renderer.h"
#include "advanced/math.h"
using namespace RMLinesRenderer;


template<typename FillFunction>
void LerpRaster<FillFunction>::operator()(Triangle t, Varyings a, Varyings b, Varyings c) {
    // Sort the triangle and the varyings
    if (t.b.y < t.a.y) {
        std::swap(t.a, t.b);
        std::swap(a, b);
    }
    if (t.c.y < t.a.y) {
        std::swap(t.a, t.c);
        std::swap(a, c);
    }
    if (t.c.y < t.b.y) {
        std::swap(t.c, t.b);
        std::swap(c, b);
    }

    float y;
    const float yaFloored = std::floor(t.a.y);
    if (t.a.y - yaFloored < 0.5f) {
        y = yaFloored + 0.5f;
    } else {
        y = yaFloored + 1.5f;
    }

    float yb;
    const float ybFloored = std::floor(t.b.y);
    if (t.b.y - ybFloored < 0.5f) {
        yb = ybFloored - 0.5f;
    } else {
        yb = ybFloored + 0.5f;
    }

    float yc;
    const float ycFloored = std::floor(t.c.y);
    if (t.c.y - ycFloored < 0.5f) {
        yc = ycFloored - 0.5f;
    } else {
        yc = ycFloored + 0.5f;
    }

    /*
     *
     *         A
     *       /   \
     *      /     \
     *     /       \
     *    /         \
     *   B   -   -   ACB
     *     --__       \
     *         --__    \
     *             --__ \
     *                 --C
     */

    if (y <= yb) {
        float difYAB = t.b.y - t.a.y;
        float difYAC = t.c.y - t.a.y;

        float dxl = (t.b.x - t.a.x) / difYAB;
        float dxr = (t.c.x - t.a.x) / difYAC;
        Varyings difXLD = (b - a) / difYAB;
        Varyings difXRD = (c - a) / difYAC;

        if (dxr < dxl) {
            std::swap(dxl, dxr);
            std::swap(difXLD, difXRD);
        }

        float height = yb - t.a.y;

        const float bx = t.a.x + dxl * height;
        const float acbx = t.a.x + dxr * height;
        float wb = acbx - bx;

        Varyings varB = a + difXLD * height;
        Varyings varACB = a + difXRD * height;
        Varyings varDX = (varACB - varB) / wb;

        float yOffset = y - t.a.y;
        float left = t.a.x + dxl * yOffset;
        float right = t.a.x + dxr * yOffset;
        Varyings varLeft = a + difXLD * yOffset;

        while (y <= yb) {
            const int l = static_cast<int>(left + 0.5f);

            if (const int r = static_cast<int>(right - 0.5f); r >= l) {
                Varyings var = varLeft + varDX * (static_cast<float>(l) + 0.5f - left);
                fill(static_cast<int>(l), static_cast<int>(y), r - l + 1, var, varDX);
            }

            left += +dxl;
            right += +dxr;
            ++y;
            varLeft = varLeft + difXLD;
        }
    }

    if (y <= yc) {
        float difYAC = t.c.y - t.a.y;
        float difYBC = t.c.y - t.b.y;

        float dxl = (t.c.x - t.b.x) / difYBC;
        float dxr = (t.c.x - t.a.x) / difYAC;
        Varyings difXLD = (c - b) / difYBC;
        Varyings difXRD = (c - a) / difYAC;

        if (dxl < dxr) {
            std::swap(dxl, dxr);
            std::swap(difXLD, difXRD);
        }

        float height = t.c.y - y;
        float left = t.c.x - dxl * height;
        float right = t.c.x - dxr * height;
        float width = right - left;

        Varyings varLeft = c - difXLD * height;
        Varyings varRight = c - difXRD * height;

        Varyings varDX = (varRight - varLeft) / width;

        while (y <= yc) {
            const int l = static_cast<int>(left + 0.5f);
            const int r = static_cast<int>(right - 0.5f);

            if (r >= l) {
                Varyings var = varLeft + varDX * (static_cast<float>(l) + 0.5f - left);
                fill(static_cast<int>(l), static_cast<int>(y), r - l + 1, var, varDX);
            }

            left += +dxl;
            right += +dxr;
            ++y;
            varLeft = varLeft + difXLD;
        }
    }
}
