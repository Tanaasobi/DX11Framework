//==============================================================================
// AnimationClip.cpp - アニメーションクリップ実装
//==============================================================================

#include "AnimationClip.h"
#include "Core/System/Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>

//==============================================================================
// キーインデックス検索
//==============================================================================
template<typename T>
int BoneAnimation::FindKeyIndex(const std::vector<T>& keys, float time)
{
	for (int i = 0; i < static_cast<int>(keys.size()) - 1; i++)
	{
		if (time < keys[i + 1].time)
		{
			return i;
		}
	}
	return static_cast<int>(keys.size()) - 2;
}

//==============================================================================
// 位置を補間取得
//==============================================================================
Vector3 BoneAnimation::GetPosition(float time) const
{
	if (positionKeys.empty())
	{
		return Vector3::Zero();
	}
	if (positionKeys.size() == 1)
	{
		return positionKeys[0].value;
	}

	int index = FindKeyIndex(positionKeys, time);
	if (index < 0) index = 0;

	int nextIndex = index + 1;
	if (nextIndex >= static_cast<int>(positionKeys.size()))
	{
		return positionKeys.back().value;
	}

	float deltaTime = positionKeys[nextIndex].time - positionKeys[index].time;
	float factor = (deltaTime > 0.0001f) ? (time - positionKeys[index].time) / deltaTime : 0.0f;
	factor = std::clamp(factor, 0.0f, 1.0f);

	return Vector3::Lerp(positionKeys[index].value, positionKeys[nextIndex].value, factor);
}

//==============================================================================
// 回転を補間取得
//==============================================================================
Quaternion BoneAnimation::GetRotation(float time) const
{
	if (rotationKeys.empty())
	{
		return Quaternion::Identity();
	}
	if (rotationKeys.size() == 1)
	{
		return rotationKeys[0].value;
	}

	int index = FindKeyIndex(rotationKeys, time);
	if (index < 0) index = 0;

	int nextIndex = index + 1;
	if (nextIndex >= static_cast<int>(rotationKeys.size()))
	{
		return rotationKeys.back().value;
	}

	float deltaTime = rotationKeys[nextIndex].time - rotationKeys[index].time;
	float factor = (deltaTime > 0.0001f) ? (time - rotationKeys[index].time) / deltaTime : 0.0f;
	factor = std::clamp(factor, 0.0f, 1.0f);

	return Quaternion::Slerp(rotationKeys[index].value, rotationKeys[nextIndex].value, factor);
}

//==============================================================================
// スケールを補間取得
//==============================================================================
Vector3 BoneAnimation::GetScale(float time) const
{
	if (scaleKeys.empty())
	{
		return Vector3::One();
	}
	if (scaleKeys.size() == 1)
	{
		return scaleKeys[0].value;
	}

	int index = FindKeyIndex(scaleKeys, time);
	if (index < 0) index = 0;

	int nextIndex = index + 1;
	if (nextIndex >= static_cast<int>(scaleKeys.size()))
	{
		return scaleKeys.back().value;
	}

	float deltaTime = scaleKeys[nextIndex].time - scaleKeys[index].time;
	float factor = (deltaTime > 0.0001f) ? (time - scaleKeys[index].time) / deltaTime : 0.0f;
	factor = std::clamp(factor, 0.0f, 1.0f);

	return Vector3::Lerp(scaleKeys[index].value, scaleKeys[nextIndex].value, factor);
}

//==============================================================================
// FBXからアニメーションをロード
//==============================================================================
bool AnimationClip::Load(const std::string& fileName)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_ConvertToLeftHanded
	);

	if (!scene)
	{
		Logger::ErrorFormat("Failed to load animation: %s", fileName.c_str());
		return false;
	}

	if (scene->mNumAnimations == 0)
	{
		Logger::ErrorFormat("No animations found in: %s", fileName.c_str());
		return false;
	}

	// 最初のアニメーションを取得
	aiAnimation* anim = scene->mAnimations[0];

	m_Name = anim->mName.C_Str();
	m_Duration = static_cast<float>(anim->mDuration);
	m_TicksPerSecond = (anim->mTicksPerSecond > 0.0) ? static_cast<float>(anim->mTicksPerSecond) : 30.0f;

	// 各ボーンのアニメーションを読み込み
	for (unsigned int i = 0; i < anim->mNumChannels; i++)
	{
		aiNodeAnim* channel = anim->mChannels[i];

		BoneAnimation boneAnim;
		boneAnim.boneName = channel->mNodeName.C_Str();

		// 位置キー
		for (unsigned int j = 0; j < channel->mNumPositionKeys; j++)
		{
			VectorKey key;
			key.time = static_cast<float>(channel->mPositionKeys[j].mTime);
			key.value.x = channel->mPositionKeys[j].mValue.x;
			key.value.y = channel->mPositionKeys[j].mValue.y;
			key.value.z = channel->mPositionKeys[j].mValue.z;
			boneAnim.positionKeys.push_back(key);
		}

		// 回転キー
		for (unsigned int j = 0; j < channel->mNumRotationKeys; j++)
		{
			QuaternionKey key;
			key.time = static_cast<float>(channel->mRotationKeys[j].mTime);
			key.value.x = channel->mRotationKeys[j].mValue.x;
			key.value.y = channel->mRotationKeys[j].mValue.y;
			key.value.z = channel->mRotationKeys[j].mValue.z;
			key.value.w = channel->mRotationKeys[j].mValue.w;
			boneAnim.rotationKeys.push_back(key);
		}

		// スケールキー
		for (unsigned int j = 0; j < channel->mNumScalingKeys; j++)
		{
			VectorKey key;
			key.time = static_cast<float>(channel->mScalingKeys[j].mTime);
			key.value.x = channel->mScalingKeys[j].mValue.x;
			key.value.y = channel->mScalingKeys[j].mValue.y;
			key.value.z = channel->mScalingKeys[j].mValue.z;
			boneAnim.scaleKeys.push_back(key);
		}

		m_BoneNameToChannel[boneAnim.boneName] = static_cast<int>(m_BoneAnimations.size());
		m_BoneAnimations.push_back(boneAnim);
	}

	Logger::InfoFormat("Animation loaded: %s (%.2f sec, %d bones)",
		fileName.c_str(), m_Duration / m_TicksPerSecond, m_BoneAnimations.size());

	return true;
}

//==============================================================================
// ボーンアニメーション取得
//==============================================================================
const BoneAnimation* AnimationClip::GetBoneAnimation(const std::string& boneName) const
{
	auto it = m_BoneNameToChannel.find(boneName);
	if (it != m_BoneNameToChannel.end())
	{
		return &m_BoneAnimations[it->second];
	}
	return nullptr;
}
