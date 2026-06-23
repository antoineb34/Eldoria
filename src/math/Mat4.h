#pragma once

#include <cmath>

#include "Vec3.h"
#include "Vec4.h"

namespace eld::math {

struct Mat4 {

    float m[4][4] {};

    static Mat4 identity() {
        Mat4 mat {};

        mat.m[0][0] = 1.0f;
        mat.m[1][1] = 1.0f;
        mat.m[2][2] = 1.0f;
        mat.m[3][3] = 1.0f;

        return mat;
    }

    static Mat4 translation(
        const Vec3& position
    ) {
        Mat4 mat = identity();

        mat.m[3][0] = position.x;
        mat.m[3][1] = position.y;
        mat.m[3][2] = position.z;

        return mat;
    }

    static Mat4 scale(
        const Vec3& value
    ) {
        Mat4 mat = identity();

        mat.m[0][0] = value.x;
        mat.m[1][1] = value.y;
        mat.m[2][2] = value.z;

        return mat;
    }

    static Mat4 rotationX(
        float radians
    ) {
        Mat4 mat = identity();

        float c = std::cos(radians);
        float s = std::sin(radians);

        mat.m[1][1] = c;
        mat.m[1][2] = s;
        mat.m[2][1] = -s;
        mat.m[2][2] = c;

        return mat;
    }

    static Mat4 rotationY(
        float radians
    ) {
        Mat4 mat = identity();

        float c = std::cos(radians);
        float s = std::sin(radians);

        mat.m[0][0] = c;
        mat.m[0][2] = -s;
        mat.m[2][0] = s;
        mat.m[2][2] = c;

        return mat;
    }

    static Mat4 rotationZ(
        float radians
    ) {
        Mat4 mat = identity();

        float c = std::cos(radians);
        float s = std::sin(radians);

        mat.m[0][0] = c;
        mat.m[0][1] = s;
        mat.m[1][0] = -s;
        mat.m[1][1] = c;

        return mat;
    }

    static Mat4 perspective(
        float fovRadians,
        float aspectRatio,
        float nearPlane,
        float farPlane
    ) {
        Mat4 mat {};

        float f =
            1.0f /
            std::tan(fovRadians * 0.5f);

        mat.m[0][0] =
            f / aspectRatio;

        mat.m[1][1] =
            f;

        mat.m[2][2] =
            farPlane /
            (farPlane - nearPlane);

        mat.m[2][3] =
            1.0f;

        mat.m[3][2] =
            (-nearPlane * farPlane) /
            (farPlane - nearPlane);

        return mat;
    }

    Mat4 operator*(
        const Mat4& other
    ) const {
        Mat4 result {};

        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 4; col++) {
                for (int i = 0; i < 4; i++) {
                    result.m[row][col] +=
                        m[row][i] *
                        other.m[i][col];
                }
            }
        }

        return result;
    }

    Vec4 transform(
        const Vec4& v
    ) const {
        return {
            v.x * m[0][0] +
            v.y * m[1][0] +
            v.z * m[2][0] +
            v.w * m[3][0],

            v.x * m[0][1] +
            v.y * m[1][1] +
            v.z * m[2][1] +
            v.w * m[3][1],

            v.x * m[0][2] +
            v.y * m[1][2] +
            v.z * m[2][2] +
            v.w * m[3][2],

            v.x * m[0][3] +
            v.y * m[1][3] +
            v.z * m[2][3] +
            v.w * m[3][3]
        };
    }

    Vec3 transformPoint(
        const Vec3& point
    ) const {
        Vec4 result =
            transform({
                point.x,
                point.y,
                point.z,
                1.0f
            });

        if (result.w != 0.0f) {
            return {
                result.x / result.w,
                result.y / result.w,
                result.z / result.w
            };
        }

        return {
            result.x,
            result.y,
            result.z
        };
    }
};

}
