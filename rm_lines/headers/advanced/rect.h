#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Vector {
    float x;
    float y;

    [[nodiscard]] json toJson() const {
        return {
            {"x", x},
            {"y", y}
        };
    }

    Vector operator+(const Vector &other) const {
        return {
            x + other.x,
            y + other.y
        };
    }

    Vector operator-(const Vector &other) const {
        return {
            x - other.x,
            y - other.y
        };
    }

    Vector operator+(const float &other) const {
        return {
            x + other,
            y + other
        };
    }

    Vector operator-(const float &other) const {
        return {
            x - other,
            y - other
        };
    }

    float halfX() const {
        return x / 2;
    }

    float halfY() const {
        return y / 2;
    }

    Vector half() const {
        return {
            x / 2,
            y / 2
        };
    }

    Vector operator*(const float m) const {
        return {
            x * m,
            y * m
        };
    }
};

struct Rect : Vector {
    float w;
    float h;

    explicit Rect(const float x = 0, const float y = 0, const float w = 0,
                  const float h = 0) : Vector(x, y), w(w), h(h) {
    }

    [[nodiscard]] json toJson() const {
        return {
            {"x", x},
            {"y", y},
            {"w", w},
            {"h", h}
        };
    }

    void setTop(const float top) {
        const float diff = top - y;
        y = top;
        h -= diff;
    }

    void setBottom(const float bottom) {
        const float diff = bottom - (y + h);
        h += diff;
    }

    void setLeft(const float left) {
        const float diff = left - x;
        x = left;
        w -= diff;
    }

    void setRight(const float right) {
        const float diff = right - (x + w);
        w += diff;
    }

    void setCenter(const Vector center) {
        const auto [x, y] = center - getCenter();
        this->x += x;
        this->y += y;
    }

    void setCenterX(const float centerX) {
        const float diff = centerX - getCenterX();
        x += diff;
    }

    void setCenterY(const float centerY) {
        const float diff = centerY - getCenterY();
        y += diff;
    }

    float getTop() const {
        return y;
    }

    float getBottom() const {
        return y + h;
    }

    float getLeft() const {
        return x;
    }

    float getRight() const {
        return x + w;
    }

    Vector getCenter() const {
        return {
            x + w / 2,
            y + h / 2
        };
    }

    float getCenterX() const {
        return x + w / 2;
    }

    float getCenterY() const {
        return y + h / 2;
    }

    static Rect fromSides(const float top, const float bottom, const float left, const float right) {
        return Rect(
            left,
            top,
            right - left,
            bottom - top
        );
    }
};
