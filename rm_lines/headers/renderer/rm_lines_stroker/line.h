#pragma once

namespace RMLinesRenderer {
    struct Line {
        float x0 = 0.0f;
        float y0 = 0.0f;
        float x1 = 0.0f;
        float y1 = 0.0f;

        float length() const;

        void normalize();
    };
}
