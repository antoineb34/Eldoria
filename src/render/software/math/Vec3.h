#pragma once

#include <cmath>

namespace rf::render {

struct Vec3 {

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3 operator+(
        const Vec3& other
    ) const {
        return {
            x + other.x,
            y + other.y,
            z + other.z
        };
    }

    Vec3 operator-(
        const Vec3& other
    ) const {
        return {
            x - other.x,
            y - other.y,
            z - other.z
        };
    }

    Vec3 operator*(
        float scalar
    ) const {
        return {
            x * scalar,
            y * scalar,
            z * scalar
        };
    }

    float dot(
        const Vec3& other
    ) const {
        return
            x * other.x +
            y * other.y +
            z * other.z;
    }

    Vec3 cross(
        const Vec3& other
    ) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    float length() const {
        return std::sqrt(
            x * x +
            y * y +
            z * z
        );
    }

    Vec3 normalized() const {
        float len = length();

        if (len == 0.0f) {
            return {};
        }

        return {
            x / len,
            y / len,
            z / len
        };
    }
};

}
