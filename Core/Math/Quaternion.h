#pragma once

//==============================================================================
// Quaternion.h - クォータニオンクラス
//==============================================================================

#include "../System/main.h"
#include "Vector3.h"

//==============================================================================
// Quaternion 構造体
//==============================================================================
struct Quaternion
{
    float x, y, z, w;

    //--------------------------------------------------------------------------
    // コンストラクタ
    //--------------------------------------------------------------------------
    Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    //--------------------------------------------------------------------------
    // 変換
    //--------------------------------------------------------------------------
    DirectX::XMVECTOR ToXMVECTOR() const
    {
        return DirectX::XMVectorSet(x, y, z, w);
    }

    static Quaternion FromXMVECTOR(DirectX::XMVECTOR v)
    {
        DirectX::XMFLOAT4 f;
        DirectX::XMStoreFloat4(&f, v);
        return Quaternion(f.x, f.y, f.z, f.w);
    }

    //--------------------------------------------------------------------------
    // 生成
    //--------------------------------------------------------------------------
    static Quaternion Identity()
    {
        return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // オイラー角から生成（ラジアン）
    static Quaternion FromEuler(float pitch, float yaw, float roll)
    {
        DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
        return FromXMVECTOR(q);
    }

    static Quaternion FromEuler(const Vector3& euler)
    {
        return FromEuler(euler.x, euler.y, euler.z);
    }

    // 軸回転から生成
    static Quaternion FromAxisAngle(const Vector3& axis, float angle)
    {
        DirectX::XMVECTOR q = DirectX::XMQuaternionRotationAxis(axis.ToXMVECTOR(), angle);
        return FromXMVECTOR(q);
    }

    //--------------------------------------------------------------------------
    // オイラー角へ変換（ラジアン）
    //--------------------------------------------------------------------------
    Vector3 ToEuler() const
    {
        Vector3 euler;

        // Roll (X軸回転)
        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        euler.x = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (Y軸回転)
        float sinp = 2.0f * (w * y - z * x);
        if (std::abs(sinp) >= 1.0f)
            euler.y = std::copysign(DirectX::XM_PIDIV2, sinp);
        else
            euler.y = std::asin(sinp);

        // Yaw (Z軸回転)
        float siny_cosp = 2.0f * (w * z + x * y);
        float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        euler.z = std::atan2(siny_cosp, cosy_cosp);

        return euler;
    }

    //--------------------------------------------------------------------------
    // 演算
    //--------------------------------------------------------------------------
    Quaternion operator*(const Quaternion& q) const
    {
        DirectX::XMVECTOR result = DirectX::XMQuaternionMultiply(ToXMVECTOR(), q.ToXMVECTOR());
        return FromXMVECTOR(result);
    }

    Quaternion& operator*=(const Quaternion& q)
    {
        *this = *this * q;
        return *this;
    }

    // ベクトルを回転
    Vector3 Rotate(const Vector3& v) const
    {
        DirectX::XMVECTOR result = DirectX::XMVector3Rotate(v.ToXMVECTOR(), ToXMVECTOR());
        DirectX::XMFLOAT3 f;
        DirectX::XMStoreFloat3(&f, result);
        return Vector3(f.x, f.y, f.z);
    }

    // 正規化
    Quaternion Normalized() const
    {
        DirectX::XMVECTOR result = DirectX::XMQuaternionNormalize(ToXMVECTOR());
        return FromXMVECTOR(result);
    }

    void Normalize()
    {
        *this = Normalized();
    }

    // 球面線形補間
    static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t)
    {
        DirectX::XMVECTOR result = DirectX::XMQuaternionSlerp(a.ToXMVECTOR(), b.ToXMVECTOR(), t);
        return FromXMVECTOR(result);
    }
};
