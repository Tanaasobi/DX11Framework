//==============================================================================
// Skeleton.cpp - スケルトン実装
//==============================================================================

#include "Skeleton.h"

using namespace DirectX;

//==============================================================================
// ボーン追加
//==============================================================================
int Skeleton::AddBone(const std::string& name, int parentIndex)
{
	int index = static_cast<int>(m_Bones.size());

	Bone bone;
	bone.name = name;
	bone.index = index;
	bone.parentIndex = parentIndex;

	m_Bones.push_back(bone);
	m_BoneNameToIndex[name] = index;

	// スキニング行列配列も拡張
	m_SkinningMatrices.resize(m_Bones.size(), XMMatrixIdentity());

	return index;
}

//==============================================================================
// ボーン取得（インデックス）
//==============================================================================
Bone* Skeleton::GetBone(int index)
{
	if (index >= 0 && index < static_cast<int>(m_Bones.size()))
	{
		return &m_Bones[index];
	}
	return nullptr;
}

//==============================================================================
// ボーン取得（名前）
//==============================================================================
Bone* Skeleton::GetBone(const std::string& name)
{
	auto it = m_BoneNameToIndex.find(name);
	if (it != m_BoneNameToIndex.end())
	{
		return &m_Bones[it->second];
	}
	return nullptr;
}

//==============================================================================
// ボーンインデックス取得
//==============================================================================
int Skeleton::GetBoneIndex(const std::string& name) const
{
	auto it = m_BoneNameToIndex.find(name);
	if (it != m_BoneNameToIndex.end())
	{
		return it->second;
	}
	return -1;
}

//==============================================================================
// 全ボーンの行列を更新
//==============================================================================
void Skeleton::UpdateBoneMatrices()
{
	// ルートから順に更新（親から子の順）
	for (int i = 0; i < static_cast<int>(m_Bones.size()); i++)
	{
		UpdateBoneWorldMatrix(i);
	}

	// スキニング行列を計算
	// スキニング行列 = オフセット行列 × ワールド行列
	for (int i = 0; i < static_cast<int>(m_Bones.size()); i++)
	{
		m_SkinningMatrices[i] = m_Bones[i].offsetMatrix * m_Bones[i].worldMatrix;
	}
}

//==============================================================================
// 個別ボーンのワールド行列を更新
//==============================================================================
void Skeleton::UpdateBoneWorldMatrix(int boneIndex)
{
	Bone& bone = m_Bones[boneIndex];

	// ローカル行列を計算
	XMMATRIX S = XMMatrixScaling(bone.localScale.x, bone.localScale.y, bone.localScale.z);
	XMMATRIX R = XMMatrixRotationQuaternion(bone.localRotation.ToXMVECTOR());
	XMMATRIX T = XMMatrixTranslation(bone.localPosition.x, bone.localPosition.y, bone.localPosition.z);
	XMMATRIX localMatrix = S * R * T;

	// 親がいれば親のワールド行列と合成
	if (bone.parentIndex >= 0)
	{
		bone.worldMatrix = localMatrix * m_Bones[bone.parentIndex].worldMatrix;
	}
	else
	{
		bone.worldMatrix = localMatrix;
	}
}

//==============================================================================
// 初期ポーズにリセット
//==============================================================================
void Skeleton::ResetToBindPose()
{
	// ボーンのローカル変換は構築時に設定された初期値のまま
	// UpdateBoneMatricesを呼んで行列を更新
	UpdateBoneMatrices();
}
