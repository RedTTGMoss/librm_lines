#pragma once
#include "clipped.h"

/*

    The clipping, if required is performed in 4 separate stages.

    To clip, we run through each side, left, right, top and bottom, and check
    the triangle. When segmenting with an edge, each triangle will be split
    into 0, 1 or 2 triangles depending. To avoid any allocations in this code
    path, we simply place new rectangles on the stack and recursively call the
    next clipXxx where Xxx is the sides in the following order: 'left',
    'right', 'top', then finally 'bottom'. From clipBottom() we call the
    actual rasterizer function.

    We do horizontal and vertical in pairs to avoid doing sorting again, and
    we perform the vertical ones last, so the output will still be vertically
    sorted triangles, which is what the rasterizer wants, so we at least won't
    have to resort them again there.

    We also include an initial check if any point is indeed outside the bounds
    and clipping is required. With the assumption unclipped polygons is the
    common-case, this gives us a tiny speedup.

*/
namespace RMLinesRenderer {
    template<typename Raster>
    void ClippedRaster<Raster>::operator()(Triangle t, Varyings a, Varyings b, Varyings c) {
        // Clipper is wrongly set up if the bounding rectangle is not normalized.
        assert(x0 <= x1);
        assert(y0 <= y1);

        if (t.a.x < x0 || t.b.x < x0 || t.c.x < x0
            || t.a.x > x1 || t.b.x > x1 || t.c.x > x1
            || t.a.y < y0 || t.b.y < y0 || t.c.y < y0
            || t.a.y > y1 || t.b.y > y1 || t.c.y > y1) {
            clipLeft(t, a, b, c);
            return;
        }

        raster(t, a, b, c);
    }


    template<typename Raster>
    void ClippedRaster<Raster>::ySort(Triangle &t, Varyings &a, Varyings &b, Varyings &c) {
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
        assert(t.a.y <= t.b.y);
        assert(t.b.y <= t.c.y);
    }


    template<typename Raster>
    void ClippedRaster<Raster>::xSort(Triangle &t, Varyings &va, Varyings &vb, Varyings &vc) {
        // Sort the triangle and the varyings
        if (t.b.x < t.a.x) {
            std::swap(t.a, t.b);
            std::swap(va, vb);
        }
        if (t.c.x < t.a.x) {
            std::swap(t.a, t.c);
            std::swap(va, vc);
        }
        if (t.c.x < t.b.x) {
            std::swap(t.c, t.b);
            std::swap(vc, vb);
        }
        assert(t.a.x <= t.b.x);
        assert(t.b.x <= t.c.x);
    }


    template<typename Raster>
    void ClippedRaster<Raster>::clipTop(Triangle t, Varyings a, Varyings b, Varyings c) {
        ySort(t, a, b, c);


        if (y0 < t.a.y) {
            clipBottom(t, a, b, c);
        } else if (y0 < t.b.y) {
            float dy = y0 - t.a.y;
            float dab = dy / (t.b.y - t.a.y);
            float dac = dy / (t.c.y - t.a.y);
            Varyings vab = a + (b - a) * dab;
            Varyings vac = a + (c - a) * dac;
            float xab = t.a.x + (t.b.x - t.a.x) * dab;
            float xac = t.a.x + (t.c.x - t.a.x) * dac;
            clipBottom(Triangle(Vector{xab, y0}, Vector{xac, y0}, t.b), vab, vac, b);
            clipBottom(Triangle(Vector{xac, y0}, t.b, t.c), vac, b, c);
        } else if (y0 < t.c.y) {
            float dy = y0 - t.c.y;
            float dca = dy / (t.a.y - t.c.y);
            float dcb = dy / (t.b.y - t.c.y);
            Varyings vca = c + (a - c) * dca;
            Varyings vcb = c + (b - c) * dcb;
            float xca = t.c.x + (t.a.x - t.c.x) * dca;
            float xcb = t.c.x + (t.b.x - t.c.x) * dcb;
            clipBottom(Triangle(Vector{xca, y0}, Vector{xcb, y0}, t.c), vca, vcb, c);
        }
    }


    template<typename Raster>
    void ClippedRaster<Raster>::clipBottom(Triangle t, Varyings a, Varyings b, Varyings c) {
        // clipBottom is called from clipTop, and it should be generating y-sorted
        // triangles already.
        assert(t.a.y <= t.b.y);
        assert(t.b.y <= t.c.y);

        if (y1 > t.c.y) {
            raster(t, a, b, c);
        } else if (y1 > t.b.y) {
            float dy = y1 - t.c.y;
            float dca = dy / (t.a.y - t.c.y);
            float dcb = dy / (t.b.y - t.c.y);
            Varyings vca = c + (a - c) * dca;
            Varyings vcb = c + (b - c) * dcb;
            float xca = t.c.x + (t.a.x - t.c.x) * dca;
            float xcb = t.c.x + (t.b.x - t.c.x) * dcb;
            raster(Triangle(t.a, t.b, Vector{xca, y1}), a, b, vca);
            raster(Triangle(t.b, Vector{xca, y1}, Vector{xcb, y1}), b, vca, vcb);
        } else if (y1 > t.a.y) {
            float dy = y1 - t.a.y;
            float dab = dy / (t.b.y - t.a.y);
            float dac = dy / (t.c.y - t.a.y);
            Varyings vab = a + (b - a) * dab;
            Varyings vac = a + (c - a) * dac;
            float xab = t.a.x + (t.b.x - t.a.x) * dab;
            float xac = t.a.x + (t.c.x - t.a.x) * dac;
            raster(Triangle(t.a, Vector{xac, y1}, Vector{xab, y1}), a, vac, vab);
        }
    }


    template<typename Raster>
    void ClippedRaster<Raster>::clipLeft(Triangle t, Varyings a, Varyings b, Varyings c) {
        xSort(t, a, b, c);

        if (x0 < t.a.x) {
            clipRight(t, a, b, c);
        } else if (x0 < t.b.x) {
            float dx = x0 - t.a.x;
            float dab = dx / (t.b.x - t.a.x);
            float dac = dx / (t.c.x - t.a.x);
            Varyings vab = a + (b - a) * dab;
            Varyings vac = a + (c - a) * dac;
            float yab = t.a.y + (t.b.y - t.a.y) * dab;
            float yac = t.a.y + (t.c.y - t.a.y) * dac;
            clipRight(Triangle(Vector{x0, yab}, Vector{x0, yac}, t.b), vab, vac, b);
            clipRight(Triangle(Vector{x0, yac}, t.b, t.c), vac, b, c);
        } else if (x0 < t.c.x) {
            float dx = x0 - t.c.x;
            float dca = dx / (t.a.x - t.c.x);
            float dcb = dx / (t.b.x - t.c.x);
            Varyings vca = c + (a - c) * dca;
            Varyings vcb = c + (b - c) * dcb;
            float yca = t.c.y + (t.a.y - t.c.y) * dca;
            float ycb = t.c.y + (t.b.y - t.c.y) * dcb;
            clipRight(Triangle(Vector{x0, yca}, Vector{x0, ycb}, t.c), vca, vcb, c);
        }
    }


    template<typename Raster>
    void ClippedRaster<Raster>::clipRight(const Triangle &t, Varyings a, Varyings b, Varyings c) {
        // We should receive sorted input from clipLeft
        assert(t.a.x <= t.b.x);
        assert(t.b.x <= t.c.x);

        if (x1 > t.c.x) {
            clipTop(t, a, b, c);
        } else if (x1 > t.b.x) {
            float dx = x1 - t.c.x;
            float dca = dx / (t.a.x - t.c.x);
            float dcb = dx / (t.b.x - t.c.x);
            Varyings vca = c + (a - c) * dca;
            Varyings vcb = c + (b - c) * dcb;
            float yca = t.c.y + (t.a.y - t.c.y) * dca;
            float ycb = t.c.y + (t.b.y - t.c.y) * dcb;
            clipTop(Triangle(t.b, Vector{x1, yca}, Vector{x1, ycb}), b, vca, vcb);
            clipTop(Triangle(t.a, t.b, Vector{x1, yca}), a, b, vca);
        } else if (x1 > t.a.x) {
            float dx = x1 - t.a.x;
            float dab = dx / (t.b.x - t.a.x);
            float dac = dx / (t.c.x - t.a.x);
            Varyings vab = a + (b - a) * dab;
            Varyings vac = a + (c - a) * dac;
            float yab = t.a.y + (t.b.y - t.a.y) * dab;
            float yac = t.a.y + (t.c.y - t.a.y) * dac;
            clipTop(Triangle(t.a, Vector{x1, yac}, Vector{x1, yab}), a, vac, vab);
        }
    }
}
