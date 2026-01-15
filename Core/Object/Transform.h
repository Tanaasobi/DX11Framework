#pragma once

//==============================================================================
// Transform.h - トランスフォームコンポーネント
//==============================================================================

#include "Component.h"
#include "../Math/Vector3.h"
#include "../Math/Quaternion.h"

//==============================================================================
// Transform クラス
// - GameObjectの位置・回転・スケールを管理
// - ワールド行列を生成
//==============================================================================
class Transform : public Component
{
public:
    Transform();
    virtual ~Transform() = default;

    //--------------------------------------------------------------------------
    // ローカル変換
    //--------------------------------------------------------------------------
    Vector3    position;    // 位置
    Quaternion rotation;    // 回転
    Vector3    scale;       // スケール

    //--------------------------------------------------------------------------
    // ワールド行列
    //--------------------------------------------------------------------------
    DirectX::XMMATRIX GetWorldMatrix() const;

    //--------------------------------------------------------------------------
    // 便利関数
    //--------------------------------------------------------------------------
    // オイラー角で回転を設定（度）
    void SetEulerAngles(float pitch, float yaw, float roll);
    void SetEulerAngles(const Vector3& euler);

    // オイラー角を取得（度）
    Vector3 GetEulerAngles() const;

    // 方向ベクトル
    Vector3 GetForward() const;
    Vector3 GetRight() const;
    Vector3 GetUp() const;

    // 移動・回転
    void Translate(const Vector3& delta);
    void Rotate(const Vector3& eulerDelta);  // 度
    void RotateAround(const Vector3& axis, float angle);  // ラジアン

    // 指定位置を向く
    void LookAt(const Vector3& target);
};
