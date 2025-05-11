#include "renderer/rm_lines_stroker/line.h"

#include <cmath>

namespace RMLinesRenderer {
    void Line::normalize() {
        const float len = length();
        const float dx = x1 - x0;
        const float dy = y1 - y0;
        x1 = x0 + dx / len;
        y1 = y0 + dy / len;
    }


    float Line::length() const {
        const float dx = x1 - x0;
        const float dy = y1 - y0;
        return std::sqrt(dx * dx + dy * dy);
    }
}
