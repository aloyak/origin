#pragma once

#include <cmath>

struct Vec4 {
    float x, y, z, w;
    Vec4(float _x = 0, float _y = 0, float _z = 0, float _w = 0) : x(_x), y(_y), z(_z), w(_w) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
    
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalize() const { float len = length(); return len > 0 ? Vec3(x / len, y / len, z / len) : *this; }

    static Vec3 lerp(const Vec3& start, const Vec3& end, float t) {
        return Vec3(
            start.x + (end.x - start.x) * t,
            start.y + (end.y - start.y) * t,
            start.z + (end.z - start.z) * t
        );
    }
};

struct Vec2 {
    float x, y;
    Vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
    
    float length() const { return std::sqrt(x*x + y*y); }
    Vec2 normalize() const { float len = length(); return len > 0 ? Vec2(x / len, y / len) : *this; }
};

struct Quat {
    float x, y, z, w;
    Quat(float _x = 0, float _y = 0, float _z = 0, float _w = 1) : x(_x), y(_y), z(_z), w(_w) {}

    static Quat fromEulerAngles(float pitch, float yaw, float roll) {
        float p = pitch * 0.5f * (3.1415926535f / 180.0f);
        float y = yaw * 0.5f * (3.1415926535f / 180.0f);
        float r = roll * 0.5f * (3.1415926535f / 180.0f);

        float sinP = std::sin(p);
        float cosP = std::cos(p);
        float sinY = std::sin(y);
        float cosY = std::cos(y);
        float sinR = std::sin(r);
        float cosR = std::cos(r);

        Quat q;
        q.w = cosR * cosP * cosY + sinR * sinP * sinY;
        q.x = sinR * cosP * cosY - cosR * sinP * sinY;
        q.y = cosR * sinP * cosY + sinR * cosP * sinY;
        q.z = cosR * cosP * sinY - sinR * sinP * cosY;
        return q;
    }

    static Quat slerp(const Quat& start, const Quat& end, float t) {
        float dot = start.x * end.x + start.y * end.y + start.z * end.z + start.w * end.w;

        Quat target = end;
        if (dot < 0.0f) {
            dot = -dot;
            target.x = -end.x;
            target.y = -end.y;
            target.z = -end.z;
            target.w = -end.w;
        }

        if (dot > 0.9995f) {
            Quat result(
                start.x + (target.x - start.x) * t,
                start.y + (target.y - start.y) * t,
                start.z + (target.z - start.z) * t,
                start.w + (target.w - start.w) * t
            );
            float len = std::sqrt(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w);
            if (len > 0.0f) {
                result.x /= len; result.y /= len; result.z /= len; result.w /= len;
            }
            return result;
        }

        float theta_0 = std::acos(dot);
        float theta = theta_0 * t;
        float sin_theta = std::sin(theta);
        float sin_theta_0 = std::sin(theta_0);

        float s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;
        float s1 = sin_theta / sin_theta_0;

        return Quat(
            (s0 * start.x) + (s1 * target.x),
            (s0 * start.y) + (s1 * target.y),
            (s0 * start.z) + (s1 * target.z),
            (s0 * start.w) + (s1 * target.w)
        );
    }
};

inline Vec3 operator*(const Vec3& a, float s) { return {a.x*s, a.y*s, a.z*s}; }
inline Vec3 operator*(float s, const Vec3& a) { return {a.x*s, a.y*s, a.z*s}; }
inline Vec3 operator/(const Vec3& a, float s) { return {a.x/s, a.y/s, a.z/s}; }
inline Vec3 operator/(float s, const Vec3& a) { return {s/a.x, s/a.y, s/a.z}; }
inline Vec3 operator*(const Vec3& a, const Vec3& b) { return {a.x*b.x, a.y*b.y, a.z*b.z}; }
inline Vec3 operator+(const Vec3& a, const Vec3& b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
inline Vec3 operator-(const Vec3& a, const Vec3& b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
inline Vec3& operator+=(Vec3& a, const Vec3& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; return a; }
inline Vec3& operator-=(Vec3& a, const Vec3& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; return a; }

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline Vec3 rotateAxisAngle(const Vec3& v, const Vec3& axis, float angle) {
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);
    return v * cosA + cross(axis, v) * sinA + axis * dot(axis, v) * (1.0f - cosA);
}

inline float length(const Vec3& v) {
    return std::sqrt(dot(v, v));
}