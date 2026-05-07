// FixedMath.h - Simple C++ Math for Doomnite
// Uses straight quotes, actual working code

#pragma once

#include <cmath>

struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    Vec3 normalize() const {
        float len = length();
        if (len < 0.0001f) return Vec3();
        return Vec3(x/len, y/len, z/len);
    }
};

inline float distance(const Vec3& a, const Vec3& b) {
    return (a - b).length();
}
