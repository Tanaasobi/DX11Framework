//==============================================================================
// Transform.cpp - トランスフォームコンポーネント実装
//==============================================================================

#include "Transform.h"
#include <algorithm>

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
Transform::Transform()
    : position(Vector3::Zero())
    , rotation(Quaternion::Identity())
    , scale(Vector3::One())
{
}

//==============================================================================
// ワールド行列を取得
//==============================================================================
XMMATRIX Transform::GetWorldMatrix() const
{
    // Scale * Rotation * Translation の順で合成
    XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
    XMMATRIX R = XMMatrixRotationQuaternion(rotation.ToXMVECTOR());
    XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);

    return S * R * T;
}

//==============================================================================
// オイラー角で回転を設定（度）
//==============================================================================
void Transform::SetEulerAngles(float pitch, float yaw, float roll)
{
    // 度からラジアンに変換
    float p = XMConvertToRadians(pitch);
    float y = XMConvertToRadians(yaw);
    float r = XMConvertToRadians(roll);

    rotation = Quaternion::FromEuler(p, y, r);
}

void Transform::SetEulerAngles(const Vector3& euler)
{
    SetEulerAngles(euler.x, euler.y, euler.z);
}

//==============================================================================
// オイラー角を取得（度）
//==============================================================================
Vector3 Transform::GetEulerAngles() const
{
    Vector3 euler = rotation.ToEuler();

    // ラジアンから度に変換
    euler.x = XMConvertToDegrees(euler.x);
    euler.y = XMConvertToDegrees(euler.y);
    euler.z = XMConvertToDegrees(euler.z);

    return euler;
}

//==============================================================================
// 方向ベクトル
//==============================================================================
Vector3 Transform::GetForward() const
{
    return rotation.Rotate(Vector3::Forward());
}

Vector3 Transform::GetRight() const
{
    return rotation.Rotate(Vector3::Right());
}

Vector3 Transform::GetUp() const
{
    return rotation.Rotate(Vector3::Up());
}

//==============================================================================
// 移動
//==============================================================================
void Transform::Translate(const Vector3& delta)
{
    position += delta;
}

//==============================================================================
// 回転（オイラー角、度）
//==============================================================================
void Transform::Rotate(const Vector3& eulerDelta)
{
    float p = XMConvertToRadians(eulerDelta.x);
    float y = XMConvertToRadians(eulerDelta.y);
    float r = XMConvertToRadians(eulerDelta.z);

    Quaternion deltaRot = Quaternion::FromEuler(p, y, r);
    rotation = rotation * deltaRot;
    rotation.Normalize();
}

//==============================================================================
// 軸回転（ラジアン）
//==============================================================================
void Transform::RotateAround(const Vector3& axis, float angle)
{
    Quaternion deltaRot = Quaternion::FromAxisAngle(axis, angle);
    rotation = rotation * deltaRot;
    rotation.Normalize();
}

//==============================================================================
// 指定位置を向く
//==============================================================================
void Transform::LookAt(const Vector3& target)
{
    Vector3 direction = (target - position).Normalized();

    if (direction.LengthSquared() < 0.0001f)
        return;

    // 前方ベクトルから回転を計算
    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR dir = direction.ToXMVECTOR();

    // 回転軸と角度を計算
    XMVECTOR axis = XMVector3Cross(forward, dir);
    float    dot = XMVectorGetX(XMVector3Dot(forward, dir));
    float    angle = std::acos(std::clamp(dot, -1.0f, 1.0f));

    if (XMVector3Length(axis).m128_f32[0] < 0.0001f)
    {
        if (dot < 0.0f)
            rotation = Quaternion::FromAxisAngle(Vector3::Up(), XM_PI);
        return;
    }

    axis = XMVector3Normalize(axis);
    XMFLOAT3 axisF;
    XMStoreFloat3(&axisF, axis);

    rotation = Quaternion::FromAxisAngle(Vector3(axisF.x, axisF.y, axisF.z), angle);
}