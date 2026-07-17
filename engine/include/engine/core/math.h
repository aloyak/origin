#pragma once

#include <cmath>

struct Vec4 {
    float x, y, z, w;
    Vec4(float _x = 0, float _y = 0, float _z = 0, float _w = 0) : x(_x), y(_y), z(_z), w(_w) {}
};

struct Mat4 {
    float m[4][4] = {};

    Mat4() { setIdentity(); }

    void setIdentity() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    // Column access: mat[col][row]
    float* operator[](int col) { return m[col]; }
    const float* operator[](int col) const { return m[col]; }

    static Mat4 perspective(float fovY_deg, float aspect, float zNear, float zFar) {
        float fovY_rad = fovY_deg * (3.1415926535f / 180.0f);
        float tanHalfFov = std::tan(fovY_rad * 0.5f);
        Mat4 result;
        result.setIdentity();
        result[0][0] = 1.0f / (aspect * tanHalfFov);
        result[1][1] = 1.0f / tanHalfFov;
        result[2][2] = -(zFar + zNear) / (zFar - zNear);
        result[2][3] = -1.0f;
        result[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
        result[3][3] = 0.0f;
        return result;
    }

    Mat4 operator*(const Mat4& b) const {
        Mat4 result;
        for (int col = 0; col < 4; col++)
            for (int row = 0; row < 4; row++) {
                result[col][row] = 0.0f;
                for (int k = 0; k < 4; k++)
                    result[col][row] += m[k][row] * b.m[col][k];
            }
        return result;
    }

    Vec4 operator*(const Vec4& v) const {
        return Vec4(
            m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0]*v.w,
            m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1]*v.w,
            m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2]*v.w,
            m[0][3]*v.x + m[1][3]*v.y + m[2][3]*v.z + m[3][3]*v.w
        );
    }
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

inline Vec2 operator-(const Vec2& a) { return Vec2(-a.x, -a.y); }
inline Vec2 operator+(const Vec2& a, const Vec2& b) { return Vec2(a.x + b.x, a.y + b.y); }
inline Vec2 operator-(const Vec2& a, const Vec2& b) { return Vec2(a.x - b.x, a.y - b.y); }
inline Vec2& operator+=(Vec2& a, const Vec2& b) { a.x += b.x; a.y += b.y; return a; }
inline Vec2& operator-=(Vec2& a, const Vec2& b) { a.x -= b.x; a.y -= b.y; return a; }

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

inline Quat inverse(const Quat& q) {
    float normSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (normSq > 0.0f) {
        float invNormSq = 1.0f / normSq;
        return Quat(-q.x * invNormSq, -q.y * invNormSq, -q.z * invNormSq, q.w * invNormSq);
    }
    return Quat(0, 0, 0, 1);
}

inline Quat operator*(const Quat& a, const Quat& b) {
    return Quat(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

inline Vec3 operator/(const Vec3& a, const Vec3& b) {
    return {a.x / b.x, a.y / b.y, a.z / b.z};
}

inline Vec3 toEulerAngles(const Quat& q) {
    Vec3 angles;

    float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    angles.x = std::atan2(sinr_cosp, cosr_cosp) * (180.0f / 3.1415926535f);

    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
    if (std::abs(sinp) >= 1.0f)
        angles.y = std::copysign(3.1415926535f / 2.0f, sinp) * (180.0f / 3.1415926535f);
    else
        angles.y = std::asin(sinp) * (180.0f / 3.1415926535f);

    float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    angles.z = std::atan2(siny_cosp, cosy_cosp) * (180.0f / 3.1415926535f);

    return angles;
}