#pragma once
#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace AdvancedMath {
    struct Vector {
        double x;
        double y;

        Vector() = default;

        template<typename T1, typename T2, typename = std::enable_if_t<
            std::is_arithmetic_v<T1> && std::is_arithmetic_v<T2>> >
        explicit Vector(T1 x, T2 y)
            : x(static_cast<double>(x)), y(static_cast<double>(y)) {
        }

        [[nodiscard]] json toJson() const {
            return {
                {"x", x},
                {"y", y}
            };
        }

        Vector operator+(const Vector &other) const {
            return Vector{
                x + other.x,
                y + other.y
            };
        }

        Vector operator-(const Vector &other) const {
            return Vector{
                x - other.x,
                y - other.y
            };
        }

        Vector operator+(const double &other) const {
            return Vector{
                x + other,
                y + other
            };
        }

        Vector operator-(const double &other) const {
            return Vector{
                x - other,
                y - other
            };
        }

        double halfX() const {
            return x / 2;
        }

        double halfY() const {
            return y / 2;
        }

        Vector half() const {
            return Vector{
                x / 2,
                y / 2
            };
        }

        Vector operator*(const double m) const {
            return Vector{
                x * m,
                y * m
            };
        }

        Vector operator/(const double d) const {
            return Vector{
                x / d,
                y / d
            };
        }

        Vector operator/(const Vector other) const {
            return Vector{
                x / other.x,
                y / other.y
            };
        }

        void operator+=(const Vector &other) {
            x += other.x;
            y += other.y;
        }

        void operator-=(const Vector &other) {
            x -= other.x;
            y -= other.y;
        }

        void operator*=(const Vector &other) {
            x *= other.x;
            y *= other.y;
        }

        void operator*=(const double &other) {
            x *= other;
            y *= other;
        }

        void operator/=(const Vector &other) {
            x /= other.x;
            y /= other.y;
        }
    };

    struct Rect : Vector {
        double w;
        double h;

        Rect() = default;

        template<typename T1, typename T2, typename T3, typename T4, typename = std::enable_if_t<
            std::is_arithmetic_v<T1> && std::is_arithmetic_v<T2> &&
            std::is_arithmetic_v<T3> && std::is_arithmetic_v<T4>> >
        explicit Rect(T1 x, T2 y, T3 w, T4 h)
            : Vector(x, y), w(static_cast<double>(w)), h(static_cast<double>(h)) {
        }

        template<typename T1, typename T2, typename = std::enable_if_t<
            std::is_arithmetic_v<T1> && std::is_arithmetic_v<T2>> >
        explicit Rect(const Vector v, T1 w, T2 h)
            : Vector(v.x, v.y), w(static_cast<double>(w)), h(static_cast<double>(h)) {
        }

        [[nodiscard]] json toJson() const {
            return {
                {"x", x},
                {"y", y},
                {"w", w},
                {"h", h}
            };
        }

        void setTop(const double top) {
            const double diff = top - y;
            y = top;
            h -= diff;
        }

        void setBottom(const double bottom) {
            const double diff = bottom - (y + h);
            h += diff;
        }

        void setLeft(const double left) {
            const double diff = left - x;
            x = left;
            w -= diff;
        }

        void setRight(const double right) {
            const double diff = right - (x + w);
            w += diff;
        }

        void setCenter(const Vector center) {
            const auto [x, y] = center - getCenter();
            this->x += x;
            this->y += y;
        }

        void setCenterX(const double centerX) {
            const double diff = centerX - getCenterX();
            x += diff;
        }

        void setCenterY(const double centerY) {
            const double diff = centerY - getCenterY();
            y += diff;
        }

        double getTop() const {
            return y;
        }

        double getBottom() const {
            return y + h;
        }

        double getLeft() const {
            return x;
        }

        double getRight() const {
            return x + w;
        }

        Vector getCenter() const {
            return Vector{
                x + w / 2,
                y + h / 2
            };
        }

        double getCenterX() const {
            return x + w / 2;
        }

        double getCenterY() const {
            return y + h / 2;
        }

        static Rect fromSides(const double top, const double bottom, const double left, const double right) {
            return Rect(
                left,
                top,
                right - left,
                bottom - top
            );
        }
    };

    struct Triangle {
        Vector a;
        Vector b;
        Vector c;

        Triangle(const Vector a, const Vector b, const Vector c) : a(a), b(b), c(c) {
        }

        Triangle(const double a0, const double a1,
                 const double b0, const double b1,
                 const double c0, const double c1) : a{a0, a1}, b{b0, b1}, c{c0, c1} {
        }

        [[nodiscard]] json toJson() const {
            return {
                {"a", a.toJson()},
                {"b", b.toJson()},
                {"c", c.toJson()}
            };
        }

        void sort() {
            if (b.y < a.y) {
                std::swap(a, b);
            }
            if (c.y < a.y) {
                std::swap(a, c);
            }
            if (c.y < b.y) {
                std::swap(c, b);
            }
        }
    };

    inline double directionToTilt(const uint32_t direction) {
        return static_cast<double>(direction) * 2 * M_PI / 255;
    }
}
