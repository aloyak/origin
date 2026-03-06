#pragma once

struct Vec3 {
    float x, y, z;
    Vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
};

struct Vec2 {
    float x, y;
    Vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
};

inline Vec3 operator*(Vec3 a, float s) { return {a.x*s, a.y*s, a.z*s}; }
inline Vec3 operator*(Vec3 a, Vec3 b) { return {a.x*b.x, a.y*b.y, a.z*b.z}; }
inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
inline Vec3& operator+=(Vec3& a, Vec3 b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; return a; }
inline Vec3& operator-=(Vec3& a, Vec3 b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; return a; }