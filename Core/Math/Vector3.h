#pragma once

//==============================================================================
// Vector3.h - 3Dベクトルクラス
//==============================================================================

#include "../System/main.h"
#include <cmath>

//==============================================================================
// Vector3 構造体
//==============================================================================
struct Vector3
{
    float x, y, z;

    //--------------------------------------------------------------------------
    // コンストラクタ
    //--------------------------------------------------------------------------
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const DirectX::XMFLOAT3& v) : x(v.x), y(v.y), z(v.z) {}

    //--------------------------------------------------------------------------
    // 変換
    //--------------------------------------------------------------------------
    DirectX::XMFLOAT3 ToXMFLOAT3() const { return DirectX::XMFLOAT3(x, y, z); }
    DirectX::XMVECTOR ToXMVECTOR() const
    {
        DirectX::XMFLOAT3 f(x, y, z);
        return DirectX::XMLoadFloat3(&f);
    }

    //--------------------------------------------------------------------------
    // 演算子
    //--------------------------------------------------------------------------
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }

    Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

    Vector3 operator-() const { return Vector3(-x, -y, -z); }

    bool operator==(const Vector3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const Vector3& v) const { return !(*this == v); }

    //--------------------------------------------------------------------------
    // ベクトル演算
    //--------------------------------------------------------------------------
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSquared() const { return x * x + y * y + z * z; }

    Vector3 Normalized() const
    {
        float len = Length();
        if (len > 0.0001f)
            return *this / len;
        return Vector3();
    }

    void Normalize()
    {
        float len = Length();
        if (len > 0.0001f)
        {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    static float Dot(const Vector3& a, const Vector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 Cross(const Vector3& a, const Vector3& b)
    {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static Vector3 Lerp(const Vector3& a, const Vector3& b, float t)
    {
        return a + (b - a) * t;
    }

    //--------------------------------------------------------------------------
    // 定数
    //--------------------------------------------------------------------------
    static Vector3 Zero() { return Vector3(0.0f, 0.0f, 0.0f); }
    static Vector3 One() { return Vector3(1.0f, 1.0f, 1.0f); }
    static Vector3 Up() { return Vector3(0.0f, 1.0f, 0.0f); }
    static Vector3 Down() { return Vector3(0.0f, -1.0f, 0.0f); }
    static Vector3 Left() { return Vector3(-1.0f, 0.0f, 0.0f); }
    static Vector3 Right() { return Vector3(1.0f, 0.0f, 0.0f); }
    static Vector3 Forward() { return Vector3(0.0f, 0.0f, 1.0f); }
    static Vector3 Back() { return Vector3(0.0f, 0.0f, -1.0f); }
};

// スカラー * ベクトル
inline Vector3 operator*(float s, const Vector3& v) { return v * s; }
