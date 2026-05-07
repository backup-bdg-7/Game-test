// MathTypes.h - C++ SIMD Math Library for Doomnite Engine
// Portable C++ with SIMD optimizations for iOS/ARM

#pragma once

#include <simd/simd.h>
#include <cmath>
#include <cstring>

namespace Doomnite {

// MARK: - Vector Types (using simd for ARM/iOS optimizations)

typedef simd_float2 Vec2;
typedef simd_float3 Vec3;
typedef simd_float4 Vec4;
typedef simd_float4x4 Mat4;
typedef simd_quatf Quat;

// MARK: - Vec3 Helpers

inline Vec3 Vec3Make(float x, float y, float z) {
    return (Vec3){x, y, z};
}

inline float Vec3Length(const Vec3& v) {
    return simd_length(v);
}

inline Vec3 Vec3Normalize(const Vec3& v) {
    return simd_normalize(v);
}

inline float Vec3Dot(const Vec3& a, const Vec3& b) {
    return simd_dot(a, b);
}

inline Vec3 Vec3Cross(const Vec3& a, const Vec3& b) {
    return simd_cross(a, b);
}

inline float Vec3Distance(const Vec3& a, const Vec3& b) {
    return simd_distance(a, b);
}

inline Vec3 Vec3Lerp(const Vec3& a, const Vec3& b, float t) {
    return simd_mix(a, b, t);
}

// Common vectors
static const Vec3 Vec3Zero = {0, 0, 0};
static const Vec3 Vec3One = {1, 1, 1};
static const Vec3 Vec3Up = {0, 1, 0};
static const Vec3 Vec3Down = {0, -1, 0};
static const Vec3 Vec3Forward = {0, 0, 1};
static const Vec3 Vec3Back = {0, 0, -1};
static const Vec3 Vec3Right = {1, 0, 0};
static const Vec3 Vec3Left = {-1, 0, 0};

// MARK: - Matrix 4x4 Helpers

inline Mat4 Mat4Identity() {
    return matrix_identity_float4x4;
}

inline Mat4 Mat4Translation(const Vec3& translation) {
    Mat4 result = matrix_identity_float4x4;
    result.columns[3] = (Vec4){translation.x, translation.y, translation.z, 1.0f};
    return result;
}

inline Mat4 Mat4Scaling(float factor) {
    return (Mat4){ {factor, 0, 0, 0}, {0, factor, 0, 0}, {0, 0, factor, 0}, {0, 0, 0, 1} };
}

inline Mat4 Mat4Scaling(const Vec3& scale) {
    return (Mat4){ {scale.x, 0, 0, 0}, {0, scale.y, 0, 0}, {0, 0, scale.z, 0}, {0, 0, 0, 1} };
}

inline Mat4 Mat4RotationX(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return (Mat4){
        (Vec4){1, 0, 0, 0},
        (Vec4){0, cosA, sinA, 0},
        (Vec4){0, -sinA, cosA, 0},
        (Vec4){0, 0, 0, 1}
    };
}

inline Mat4 Mat4RotationY(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return (Mat4){
        (Vec4){cosA, 0, -sinA, 0},
        (Vec4){0, 1, 0, 0},
        (Vec4){sinA, 0, cosA, 0},
        (Vec4){0, 0, 0, 1}
    };
}

inline Mat4 Mat4RotationZ(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return (Mat4){
        (Vec4){cosA, sinA, 0, 0},
        (Vec4){-sinA, cosA, 0, 0},
        (Vec4){0, 0, 1, 0},
        (Vec4){0, 0, 0, 1}
    };
}

inline Mat4 Mat4Perspective(float fovY, float aspect, float near, float far) {
    float f = 1.0f / tanf(fovY * 0.5f);
    float nf = 1.0f / (near - far);
    
    Mat4 result = {0};
    result.columns[0] = (Vec4){f / aspect, 0, 0, 0};
    result.columns[1] = (Vec4){0, f, 0, 0};
    result.columns[2] = (Vec4){0, 0, (far + near) * nf, -1};
    result.columns[3] = (Vec4){0, 0, 2 * far * near * nf, 0};
    return result;
}

inline Mat4 Mat4LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 z = simd_normalize(eye - center);
    Vec3 x = simd_normalize(simd_cross(up, z));
    Vec3 y = simd_cross(z, x);
    
    Mat4 result = matrix_identity_float4x4;
    result.columns[0] = (Vec4){x.x, y.x, z.x, 0};
    result.columns[1] = (Vec4){x.y, y.y, z.y, 0};
    result.columns[2] = (Vec4){x.z, y.z, z.z, 0};
    result.columns[3] = (Vec4){-simd_dot(x, eye), -simd_dot(y, eye), -simd_dot(z, eye), 1};
    return result;
}

// MARK: - Quaternion Helpers

inline Quat QuatIdentity() {
    return (Quat){0, 0, 0, 1};
}

inline Quat QuatFromEuler(float pitch, float yaw, float roll) {
    return simd_quaternion_from_euler(pitch, yaw, roll);
}

inline Vec3 QuatToEuler(const Quat& q) {
    return simd_euler_angles(q);
}

// MARK: - Math Utilities

class MathUtils {
public:
    static const float PI;
    static const float TWO_PI;
    static const float HALF_PI;
    static const float EPSILON;
    
    static float DegreesToRadians(float degrees) {
        return degrees * PI / 180.0f;
    }
    
    static float RadiansToDegrees(float radians) {
        return radians * 180.0f / PI;
    }
    
    template<typename T>
    static T Clamp(T value, T minVal, T maxVal) {
        return value < minVal ? minVal : (value > maxVal ? maxVal : value);
    }
    
    static float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    static float Smoothstep(float t) {
        return t * t * (3.0f - 2.0f * t);
    }
};

// Static member definitions
const float MathUtils::PI = 3.14159265358979323846f;
const float MathUtils::TWO_PI = PI * 2.0f;
const float MathUtils::HALF_PI = PI * 0.5f;
const float MathUtils::EPSILON = 0.0001f;

} // namespace Doomnite
