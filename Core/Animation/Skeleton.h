#pragma once

//==============================================================================
// Skeleton.h - スケルトン（ボーン階層）
//==============================================================================

#include "Bone.h"
#include <vector>
#include <unordered_map>

//==============================================================================
// Skeleton クラス
// - ボーンの階層構造を管理
// - ボーン行列の計算
//==============================================================================
class Skeleton
{
public:
	Skeleton() = default;
	~Skeleton() = default;

	// ボーン追加
	int AddBone(const std::string& name, int parentIndex = -1);

	// ボーン取得
	Bone* GetBone(int index);
	Bone* GetBone(const std::string& name);
	int GetBoneIndex(const std::string& name) const;

	// ボーン数
	size_t GetBoneCount() const { return m_Bones.size(); }

	// 全ボーンの行列を更新
	void UpdateBoneMatrices();

	// スキニング行列配列を取得（シェーダーに送る用）
	const std::vector<DirectX::XMMATRIX>& GetSkinningMatrices() const { return m_SkinningMatrices; }

	// ボーン配列への直接アクセス
	std::vector<Bone>& GetBones() { return m_Bones; }
	const std::vector<Bone>& GetBones() const { return m_Bones; }

	// 初期ポーズにリセット
	void ResetToBindPose();

private:
	std::vector<Bone> m_Bones;
	std::unordered_map<std::string, int> m_BoneNameToIndex;
	std::vector<DirectX::XMMATRIX> m_SkinningMatrices;

	void UpdateBoneWorldMatrix(int boneIndex);
};
