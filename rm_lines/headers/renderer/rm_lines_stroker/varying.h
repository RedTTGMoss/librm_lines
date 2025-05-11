#pragma once

namespace RMLinesRenderer {
    struct [[nodiscard]] VaryingNoop {
        // ReSharper disable once CppNonExplicitConvertingConstructor
        VaryingNoop(float = 0.0, float = 0.0) {
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
}
