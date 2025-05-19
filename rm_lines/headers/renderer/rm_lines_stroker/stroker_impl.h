#pragma once
#include "advanced/math.h"
#include <cassert>
#include <cmath>
#include <numbers>
#include <optional>


#include "renderer/rm_lines_stroker/stroker.h"

using Vector = AdvancedMath::Vector;
using Triangle = AdvancedMath::Triangle;

namespace RMLinesRenderer {
    template<typename Rasterizer, typename VaryingGenerator>
    Stroker<Rasterizer, VaryingGenerator>::Segment::Segment(const SegmentType type,
                                                            const float x, const float y, const float width,
                                                            const float length,
                                                            const JoinStyle joinStyle, const CapStyle capStyle,
                                                            Varyings left, Varyings right)
        : x(x)
          , y(y)
          , width(width)
          , length(length)
          , leftVarying(left)
          , rightVarying(right)
          , type(type)
          , joinStyle(joinStyle)
          , capStyle(capStyle) {
    }


    template<typename Rasterizer, typename VaryingGenerator>
    Stroker<Rasterizer, VaryingGenerator>::~Stroker() {
        finish();
    }


    template<typename Rasterizer, typename VaryingGenerator>
    Stroker<Rasterizer, VaryingGenerator>::Stroker() {
        reset();
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer,
        VaryingGenerator>::store(float x, float y, SegmentType type, Varyings left, Varyings right) {
        m_lastSegment = Segment(type,
                                x, y, width, length,
                                joinStyle,
                                capStyle,
                                left, right);
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::moveTo(const float x, const float y) {
        if (m_lastSegment.type == LineToSegment) {
            flushStartCap();
            flushEndCap();
        }

        store(x, y, MoveToSegment, varying.left(length, width / 2), varying.right(length, width / 2));
        m_firstSegment = m_lastSegment;
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::lineTo(float x, float y) {
        if (m_lastSegment.type == InvalidType) {
            moveTo(x, y);
            return;
        }

        if (m_lastSegment.x == x && m_lastSegment.y == y)
            return;

        if (slant.has_value()) {
            lineToSlanted(x, y);
            return;
        }

        const Line line{m_lastSegment.x, m_lastSegment.y, x, y};
        const float len = line.length();
        const float ndx = (line.y0 - line.y1) / len;
        const float ndy = (line.x1 - line.x0) / len;

        float chw = width / 2;
        const float lhw = m_lastSegment.width / 2;


        const Line right{
            line.x0 + ndx * lhw,
            line.y0 + ndy * lhw,
            line.x1 + ndx * chw,
            line.y1 + ndy * chw
        };
        const Line left{
            line.x0 - ndx * lhw,
            line.y0 - ndy * lhw,
            line.x1 - ndx * chw,
            line.y1 - ndy * chw
        };

        if (m_lastSegment.type == LineToSegment) {
            join(m_lastLeft, m_lastRight, left, right, m_lastSegment.leftVarying, m_lastSegment.rightVarying);
        }

        length += len;
        Varyings leftVarying = varying.left(length, chw);
        Varyings rightVarying = varying.right(length, chw);

        stroke(left, right,
               m_lastSegment.leftVarying, m_lastSegment.rightVarying,
               leftVarying, rightVarying);

        if (m_lastSegment.type == MoveToSegment) {
            m_firstLeft = left;
            m_firstRight = right;
        }

        m_lastLeft = left;
        m_lastRight = right;
        store(x, y, LineToSegment, leftVarying, rightVarying);
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::lineToSlanted(float x, float y) {
        assert(m_lastSegment.type != InvalidType);
        assert(m_lastSegment.x != x || m_lastSegment.y != y);
        assert(slant.has_value());

        const Line line{m_lastSegment.x, m_lastSegment.y, x, y};
        const float len = line.length();

        // line normal
        const float ndx = (line.y0 - line.y1) / len;
        const float ndy = (line.x1 - line.x0) / len;

        // current and last half-width
        const float chw = width / 2;
        const float lhw = m_lastSegment.width / 2;

        // Figure out how wide the line ends up being. The width stands normal to
        // the current line direction, so we can find that by taking finding the
        // dot product between the slant vector and the line normal.
        const float slantDotNormal = ndx * slant->nx + ndy * slant->ny;
        const float effectiveWidth = std::abs(slantDotNormal) * width;

        Line right{
            line.x0 + slant->nx * lhw,
            line.y0 + slant->ny * lhw,
            line.x1 + slant->nx * chw,
            line.y1 + slant->ny * chw
        };
        Line left{
            line.x0 - slant->nx * lhw,
            line.y0 - slant->ny * lhw,
            line.x1 - slant->nx * chw,
            line.y1 - slant->ny * chw
        };

        if (effectiveWidth < slant->minimumWidth) {
            const float mw2 = slant->minimumWidth / 2.0f;
            right = Line{
                line.x0 + ndx * mw2,
                line.y0 + ndy * mw2,
                line.x1 + ndx * mw2,
                line.y1 + ndy * mw2
            };
            left = Line{
                line.x0 - ndx * mw2,
                line.y0 - ndy * mw2,
                line.x1 - ndx * mw2,
                line.y1 - ndy * mw2
            };
        }

        if (m_lastSegment.type == LineToSegment) {
            join(m_lastLeft, m_lastRight, left, right, m_lastSegment.leftVarying, m_lastSegment.rightVarying);
        }

        length += len;
        Varyings leftVarying = varying.left(length, chw);
        Varyings rightVarying = varying.right(length, chw);

        stroke(left, right,
               m_lastSegment.leftVarying, m_lastSegment.rightVarying,
               leftVarying, rightVarying);

        if (m_lastSegment.type == MoveToSegment) {
            m_firstLeft = left;
            m_firstRight = right;
        }

        m_lastLeft = left;
        m_lastRight = right;
        store(x, y, LineToSegment, leftVarying, rightVarying);
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::join(Line lastLeft, const Line lastRight, Line left, const Line right,
                                                     Varyings leftVarying, Varyings rightVarying) {
        if (joinStyle == BevelJoin) {
            stroke(Line{lastLeft.x1, lastLeft.y1, left.x0, left.y0},
                   Line{lastRight.x1, lastRight.y1, right.x0, right.y0},
                   leftVarying, rightVarying,
                   leftVarying, rightVarying,
                   true);
        } else if (joinStyle == RoundJoin) {
            const float angleLast = std::atan2(lastLeft.y1 - m_lastSegment.y, lastLeft.x1 - m_lastSegment.x);
            const float angleNext = std::atan2(left.y0 - m_lastSegment.y, left.x0 - m_lastSegment.x);
            float angleDelta = angleNext - angleLast;
            if (angleDelta < -M_PI) {
                angleDelta += M_PI * 2;
            } else if (angleDelta > M_PI) {
                angleDelta -= M_PI * 2;
            }

            // Shortcut with a bevel join for 'tiny' angles.
            // ### premature-optimization: Need to verify that this makes sense
            // per-wise and that it at the same time doesn't cause too much of a visual
            // impact...
            if (std::abs(angleDelta) < M_PI / 10.0f) {
                stroke(Line{left.x0, left.y0, lastLeft.x1, lastLeft.y1},
                       Line{right.x0, right.y0, lastRight.x1, lastRight.y1},
                       leftVarying, rightVarying,
                       leftVarying, rightVarying,
                       true);
                return;
            }

            // Decide the radius based on the previous segment. We're joining from
            // it, and if the width is changing, 'width' will already have been set
            // to the new value.
            const float radius = m_lastSegment.width / 2;
            const float arcLength = radius * angleDelta; // from (angleDelta / (2 * PI)) * (2 * PI * r)

            // Don't really know how long steps we have to take, but let's assume a
            // bit more than 3 gives us good results... Hey, lets just use PI.
            const int steps =
                    std::min(30, static_cast<int>(std::ceil(std::abs(arcLength / M_PI))));
            assert(steps > 0);

            float llx = lastLeft.x1;
            float lly = lastLeft.y1;
            float lrx = lastRight.x1;
            float lry = lastRight.y1;
            const float dt = angleDelta / static_cast<float>(steps);
            float t = angleLast + dt;
            for (int i = 0; i < steps; ++i) {
                float ct = radius * std::cos(t);
                float st = radius * std::sin(t);
                const float lx = m_lastSegment.x + ct;
                const float ly = m_lastSegment.y + st;
                const float rx = m_lastSegment.x - ct;
                const float ry = m_lastSegment.y - st;

                stroke(Line{llx, lly, lx, ly},
                       Line{lrx, lry, rx, ry},
                       leftVarying, rightVarying,
                       leftVarying, rightVarying,
                       true);

                t += dt;
                llx = lx;
                lly = ly;
                lrx = rx;
                lry = ry;
            }
        }
    }

    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::cap(Line left, Line right, Segment s, const bool endCap) {
        if (s.width <= 0.0f)
            return;

        if (capStyle == FlatCap) {
            return;
        }

        if (!endCap) {
            const Line tmp{left.x1, left.y1, left.x0, left.y0};
            left = Line{right.x1, right.y1, right.x0, right.y0};
            right = tmp;
        }

        if (capStyle == RoundCap) {
            const float angle = std::atan2(left.y1 - s.y, left.x1 - s.x);

            const float radius = s.width / 2;
            const float arcLength = radius * M_PI; // half a circle...
            int steps = std::min(30, static_cast<int>(std::ceil(std::abs(arcLength / M_PI))));
            assert(steps > 0);

            float lx = left.x1;
            float ly = left.y1;
            float rx = right.x1;
            float ry = right.y1;

            const float dt = M_PI / steps;
            float rt = angle + static_cast<float>(M_PI - dt);
            float lt = angle + dt;

            steps = std::max(1, steps / 2);

            for (int i = 0; i < steps; ++i) {
                const float nlx = radius * std::cos(lt) + s.x;
                const float nly = radius * std::sin(lt) + s.y;
                const float nrx = radius * std::cos(rt) + s.x;
                const float nry = radius * std::sin(rt) + s.y;

                stroke(Line{lx, ly, nlx, nly},
                       Line{rx, ry, nrx, nry},
                       s.leftVarying, s.rightVarying,
                       s.leftVarying, s.rightVarying);

                lx = nlx;
                ly = nly;
                lt += dt;
                rx = nrx;
                ry = nry;
                rt -= dt;
            }
        } else if (capStyle == SquareCap) {
            const float w2 = width / 2.0f;

            const float leftDifX = left.x1 - left.x0;
            const float leftDifY = left.y1 - left.y0;
            const float leftLen = std::sqrt(leftDifX * leftDifX + leftDifY * leftDifY);
            const float normalizedLeftDifX = leftDifX / leftLen;
            const float normalizedLeftDifY = leftDifY / leftLen;
            const float lx = left.x1 + normalizedLeftDifX * w2;
            const float ly = left.y1 + normalizedLeftDifY * w2;

            const float rightDifX = right.x1 - right.x0;
            const float rightDifY = right.y1 - right.y0;
            const float rightLen = std::sqrt(rightDifX * rightDifX + rightDifY * rightDifY);
            const float normalizedRightDifX = rightDifX / rightLen;
            const float normalizedRightDifY = rightDifY / rightLen;
            const float rx = right.x1 + normalizedRightDifX * w2;
            const float ry = right.y1 + normalizedRightDifY * w2;

            stroke(Line{left.x1, left.y1, lx, ly},
                   Line{right.x1, right.y1, rx, ry},
                   s.leftVarying, s.rightVarying,
                   s.leftVarying, s.rightVarying);
        }
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::flushStartCap() {
        if (!m_startCapRendered && m_lastSegment.type == LineToSegment) {
            cap(m_firstLeft, m_firstRight, m_firstSegment, false);
            m_startCapRendered = true;
        }
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::flushEndCap() {
        if (!m_endCapRendered && m_lastSegment.type == LineToSegment) {
            cap(m_lastLeft, m_lastRight, m_lastSegment, true);
            m_endCapRendered = true;
        }
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::close() {
        if (m_lastSegment.type == LineToSegment) {
            assert(m_firstSegment.type == MoveToSegment);
            lineTo(m_firstSegment.x, m_firstSegment.y);

            join(m_lastLeft, m_lastRight, m_firstLeft, m_firstRight,
                 m_lastSegment.leftVarying, m_lastSegment.rightVarying);
        }
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::finish() {
        if (m_lastSegment.type == LineToSegment) {
            if (m_lastSegment.x == m_firstSegment.x && m_lastSegment.y == m_firstSegment.y) {
                close();
            } else {
                flushStartCap();
                flushEndCap();
            }
        }

        reset();
    }


    template<typename Rasterizer, typename VaryingGenerator>
    void Stroker<Rasterizer, VaryingGenerator>::reset() {
        m_firstLeft = Line{};
        m_firstRight = Line{};
        m_lastSegment = Segment();
        m_firstSegment = Segment();
        triangleCount = 0;
        length = 0;
        m_startCapRendered = false;
        m_endCapRendered = false;
    }

    template<typename Rasterizer, typename VaryingGenerator>
    auto Stroker<Rasterizer, VaryingGenerator>::stroke(const Line left, Line right,
                                                       Varyings lastLeftVarying, Varyings lastRightVarying,
                                                       Varyings leftVarying, Varyings rightVarying,
                                                       const bool checkDirection) -> void {
        if (checkDirection) {
            // Catch the case where we've decided to stroke lines that pass in
            // opposite directions. This typically happens whenever we join two line
            // segments as the interior of the curve is wrapped back on itself then.

            // We do this by projecting right onto left and checking the
            // direction. Since we only care about the sign and not the length, we
            // can skip the sqrt() and simplify the function a bit

            if ((right.x1 - right.x0) * (left.x1 - left.x0) + (right.y1 - right.y0) * (left.y1 - left.y0) < 0) {
                // If opposite, flip the right line and its varyings...
                right = Line{right.x1, right.y1, right.x0, right.y0};
                std::swap(lastRightVarying, rightVarying);
            }
        }

        raster(Triangle(Vector(left.x0, left.y0),
                        Vector(left.x1, left.y1),
                        Vector(right.x0, right.y0)),
               lastLeftVarying, leftVarying, lastRightVarying);

        raster(Triangle(Vector(right.x0, right.y0),
                        Vector(left.x1, left.y1),
                        Vector(right.x1, right.y1)),
               lastRightVarying, leftVarying, rightVarying);

        triangleCount += 2;
    }
}
