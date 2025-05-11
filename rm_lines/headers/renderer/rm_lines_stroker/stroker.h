#pragma once
#include <cmath>
#include <cstdint>
#include <optional>

#include "line.h"
#include "varying.h"

namespace RMLinesRenderer {
    enum CapStyle : uint8_t {
        FlatCap,
        SquareCap,
        RoundCap
    };

    enum JoinStyle : uint8_t {
        BevelJoin,
        // MiterJoin, // not implemented, so lets not pretend
        RoundJoin
    };

    struct Slant {
        float nx = 0.0f;
        float ny = 0.0f;
        float minimumWidth = 0.0f;

        bool isValid() const {
            constexpr float almostZero = 0.0001f;
            return std::abs(std::sqrt(nx * nx + ny * ny) - 1.0f) < almostZero;
        }
    };

    template<typename Rasterizer, typename VaryingGenerator = VaryingGeneratorNoop>
    struct Stroker {
        JoinStyle joinStyle = BevelJoin;
        CapStyle capStyle = FlatCap;
        float width = 1.0f;

        std::optional<Slant> slant;

        Rasterizer raster;
        VaryingGenerator varying;

        int triangleCount = 0;
        float length = 0.0f;

        Stroker(); //del

        ~Stroker(); //del

        void moveTo(float x, float y); //del

        void lineTo(float x, float y); //del

        void close(); //del

        void finish(); //del

        void reset(); //del

        void flushStartCap(); //del

        void flushEndCap(); //del

        // ********************
        // Internals
        //

        using Varyings = typename Rasterizer::Varyings;

        enum SegmentType : uint8_t {
            InvalidType,
            MoveToSegment,
            LineToSegment
        };

        struct Segment {
            float x;
            float y;
            float width;
            float length;

            Varyings leftVarying;
            Varyings rightVarying;

            SegmentType type;
            JoinStyle joinStyle;
            CapStyle capStyle;

            explicit Segment(SegmentType type = InvalidType,
                             float x = 0.0f,
                             float y = 0.0f,
                             float width = 1.0f,
                             float length = 0.0f,
                             JoinStyle joinStyle = BevelJoin,
                             CapStyle capStyle = FlatCap,
                             Varyings left = Varyings(),
                             Varyings right = Varyings()); //del
        };

        void lineToSlanted(float x, float y); //del

        void cap(Line left, Line right, Segment s, bool endCap); //del

        void store(float x, float y, SegmentType type, Varyings left, Varyings right); //del

        void join(Line lastLeft, Line lastRight, Line left, Line right, Varyings leftVarying,
                  Varyings rightVarying); //del

        void stroke(Line left, Line right,
                    Varyings lastLeftVarying, Varyings lastRightVarying,
                    Varyings leftVarying, Varyings rightVarying,
                    bool checkDirection = false); //del

        Segment m_lastSegment;
        Line m_lastLeft;
        Line m_lastRight;

        Segment m_firstSegment;
        Line m_firstLeft;
        Line m_firstRight;

        bool m_startCapRendered = false;
        bool m_endCapRendered = false;
    };
}
