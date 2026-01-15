#pragma once

//==============================================================================
// Bone.h - ボーン構造体
//==============================================================================

#include "Core/System/main.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Quaternion.h"
#include <string>

//==============================================================================
// ボーンの最大数（シェーダーの定数バッファサイズに影響）
//==============================================================================
constexpr int MAX_BONES = 256;

//==============================================================================
// Bone 構造体
//==============================================================================
struct Bone
{
	std::string name;
	int         index = -1;
	int         parentIndex = -1;

	// オフセット行列（メッシュ空間 → ボーン空間）
	DirectX::XMMATRIX offsetMatrix = DirectX::XMMatrixIdentity();

	// ローカル変換
	Vector3    localPosition;
	Quaternion localRotation;
	Vector3    localScale = Vector3::One();

	// 計算されたワールド行列
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

	// 最終的なスキニング行列
	DirectX::XMMATRIX skinningMatrix = DirectX::XMMatrixIdentity();
};
