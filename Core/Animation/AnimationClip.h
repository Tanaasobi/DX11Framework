#pragma once

//==============================================================================
// AnimationClip.h - アニメーションクリップ
//==============================================================================

#include "Core/System/main.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Quaternion.h"
#include <string>
#include <vector>
#include <unordered_map>

//==============================================================================
// キーフレーム構造体
//==============================================================================
struct VectorKey
{
	float   time;
	Vector3 value;
};

struct QuaternionKey
{
	float      time;
	Quaternion value;
};

//==============================================================================
// ボーンアニメーションチャンネル
//==============================================================================
struct BoneAnimation
{
	std::string boneName;
	std::vector<VectorKey>     positionKeys;
	std::vector<QuaternionKey> rotationKeys;
	std::vector<VectorKey>     scaleKeys;

	// 指定時間の値を補間して取得
	Vector3    GetPosition(float time) const;
	Quaternion GetRotation(float time) const;
	Vector3    GetScale(float time) const;

private:
	template<typename T>
	static int FindKeyIndex(const std::vector<T>& keys, float time);
};

//==============================================================================
// AnimationClip クラス
// - 1つのアニメーションデータを保持
//==============================================================================
class AnimationClip
{
public:
	AnimationClip() = default;
	~AnimationClip() = default;

	// FBXからアニメーションをロード
	bool Load(const std::string& fileName);

	// アクセサ
	const std::string& GetName() const { return m_Name; }
	void SetName(const std::string& name) { m_Name = name; }

	float GetDuration() const { return m_Duration; }
	float GetTicksPerSecond() const { return m_TicksPerSecond; }

	// ボーンアニメーション取得
	const BoneAnimation* GetBoneAnimation(const std::string& boneName) const;

	// 全ボーンアニメーション
	const std::vector<BoneAnimation>& GetBoneAnimations() const { return m_BoneAnimations; }

private:
	std::string m_Name;
	float m_Duration = 0.0f;
	float m_TicksPerSecond = 30.0f;

	std::vector<BoneAnimation> m_BoneAnimations;
	std::unordered_map<std::string, int> m_BoneNameToChannel;
};
