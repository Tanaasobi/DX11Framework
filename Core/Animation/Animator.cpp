//==============================================================================
// Animator.cpp - アニメーターコンポーネント実装
//==============================================================================

#include "Animator.h"
#include "Core/System/Logger.h"
#include <algorithm>

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
Animator::Animator()
{
	for (int i = 0; i < MAX_LAYERS; i++)
	{
		m_Layers[i] = {};
		m_FadeOutLayers[i] = {};
		m_FadeTime[i] = 0.0f;
		m_FadeElapsed[i] = 0.0f;
		m_IsFading[i] = false;
	}

	for (int i = 0; i < MAX_LAYERS; i++)
	{
		m_ReturnClip[i] = nullptr;
		m_WaitingReturn[i] = false;
	}
}

//==============================================================================
// デストラクタ
//==============================================================================
Animator::~Animator()
{
}

//==============================================================================
// 更新
//==============================================================================
void Animator::Update(float deltaTime)
{
	if (!m_Skeleton) return;

	// 各レイヤーの更新
	for (int i = 0; i < MAX_LAYERS; i++)
	{
		AnimationLayer& layer = m_Layers[i];

		if (!layer.clip) continue;

		// 時間更新
		layer.time += deltaTime * layer.speed;

		float duration = layer.clip->GetDuration() / layer.clip->GetTicksPerSecond();

		// デバッグログ
		if (!layer.loop)
		{
			Logger::InfoFormat("Layer %d: time=%.2f, duration=%.2f, loop=%d, waiting=%d",
				i, layer.time, duration, layer.loop, m_WaitingReturn[i]);
		}

		if (layer.time >= duration)
		{
			if (layer.loop)
			{
				layer.time = fmod(layer.time, duration);
			}
			else
			{
				layer.time = duration;

				Logger::Info("Animation finished! Checking return...");

				if (m_WaitingReturn[i] && m_ReturnClip[i])
				{
					Logger::Info("Returning to previous animation");
					AnimationClip* returnClip = m_ReturnClip[i];
					m_ReturnClip[i] = nullptr;
					m_WaitingReturn[i] = false;
					CrossFade(returnClip, 0.15f, i);
				}
				else
				{
					Logger::InfoFormat("No return clip: waiting=%d, clip=%p",
						m_WaitingReturn[i], m_ReturnClip[i]);
				}
			}
		}

		// フェード更新
		if (m_IsFading[i])
		{
			m_FadeElapsed[i] += deltaTime;
			if (m_FadeElapsed[i] >= m_FadeTime[i])
			{
				m_IsFading[i] = false;
				m_FadeOutLayers[i].clip = nullptr;
			}
		}
	}

	// ボーン行列計算
	BlendBones();
}
//==============================================================================
// スケルトン設定
//==============================================================================
void Animator::SetSkeleton(std::shared_ptr<Skeleton> skeleton)
{
	m_Skeleton = skeleton;
}

//==============================================================================
// アニメーション再生
//==============================================================================
void Animator::Play(AnimationClip* clip, int layer, float fadeTime)
{
	if (layer < 0 || layer >= MAX_LAYERS) return;

	if (fadeTime > 0.0f && m_Layers[layer].clip)
	{
		CrossFade(clip, fadeTime, layer);
	}
	else
	{
		m_Layers[layer].clip = clip;
		m_Layers[layer].time = 0.0f;
		m_Layers[layer].weight = 1.0f;
		m_Layers[layer].speed = 1.0f;
		m_Layers[layer].loop = true;
		m_IsFading[layer] = false;
	}
}

//==============================================================================
// 1回だけ再生（ループなし）
//==============================================================================
void Animator::PlayOnce(AnimationClip* clip, float blendTime, float maxDuration)
{
	if (!clip) return;

	int layer = 0;

	// 現在のアニメーションを保存
	if (m_Layers[layer].clip && m_Layers[layer].clip != clip)
	{
		m_ReturnClip[layer] = m_Layers[layer].clip;
		m_WaitingReturn[layer] = true;
	}

	// フェード設定
	if (blendTime > 0.0f && m_Layers[layer].clip)
	{
		m_FadeOutLayers[layer] = m_Layers[layer];
		m_FadeTime[layer] = blendTime;
		m_FadeElapsed[layer] = 0.0f;
		m_IsFading[layer] = true;
	}
	else
	{
		m_IsFading[layer] = false;
	}

	// 新しいアニメーション
	m_Layers[layer].clip = clip;
	m_Layers[layer].time = 0.0f;
	m_Layers[layer].loop = false;
	m_Layers[layer].weight = 1.0f;

	// 最大時間が指定されていれば速度を調整
	if (maxDuration > 0.0f)
	{
		float duration = clip->GetDuration();
		if (duration > maxDuration)
		{
			// アニメーション速度を上げて指定時間内に収める
			m_Layers[layer].speed = duration / maxDuration;
		}
		else
		{
			m_Layers[layer].speed = 1.0f;
		}
	}
	else
	{
		m_Layers[layer].speed = 1.0f;
	}
}


//==============================================================================
// 停止
//==============================================================================
void Animator::Stop(int layer)
{
	if (layer < 0 || layer >= MAX_LAYERS) return;

	m_Layers[layer].clip = nullptr;
	m_Layers[layer].time = 0.0f;
	m_IsFading[layer] = false;
}

//==============================================================================
// レイヤーウェイト設定
//==============================================================================
void Animator::SetLayerWeight(int layer, float weight)
{
	if (layer < 0 || layer >= MAX_LAYERS) return;
	m_Layers[layer].weight = std::clamp(weight, 0.0f, 1.0f);
}

//==============================================================================
// レイヤースピード設定
//==============================================================================
void Animator::SetLayerSpeed(int layer, float speed)
{
	if (layer < 0 || layer >= MAX_LAYERS) return;
	m_Layers[layer].speed = speed;
}

//==============================================================================
// ループ設定
//==============================================================================
void Animator::SetLooping(int layer, bool loop)
{
	if (layer < 0 || layer >= MAX_LAYERS) return;
	m_Layers[layer].loop = loop;
}

//==============================================================================
// 現在時間取得
//==============================================================================
float Animator::GetTime(int layer) const
{
	if (layer < 0 || layer >= MAX_LAYERS) return 0.0f;
	return m_Layers[layer].time;
}

//==============================================================================
// 正規化時間取得
//==============================================================================
float Animator::GetNormalizedTime(int layer) const
{
	if (layer < 0 || layer >= MAX_LAYERS) return 0.0f;

	const AnimationLayer& l = m_Layers[layer];
	if (!l.clip) return 0.0f;

	float duration = l.clip->GetDuration() / l.clip->GetTicksPerSecond();
	return (duration > 0.0f) ? (l.time / duration) : 0.0f;
}

//==============================================================================
// 再生中かどうか
//==============================================================================
bool Animator::IsPlaying(int layer) const
{
	if (layer < 0 || layer >= MAX_LAYERS) return false;
	return m_Layers[layer].clip != nullptr;
}

//==============================================================================
// 指定アニメーションが再生中か
//==============================================================================
bool Animator::IsPlaying(AnimationClip* clip) const
{
	if (!clip) return false;

	for (int i = 0; i < MAX_LAYERS; i++)
	{
		if (m_Layers[i].clip == clip)
		{
			// ループしない場合は終了チェック
			if (!m_Layers[i].loop)
			{
				float duration = clip->GetDuration();
				if (m_Layers[i].time >= duration)
				{
					return false;  // 終了済み
				}
			}
			return true;
		}
	}

	return false;
}


//==============================================================================
// クロスフェード
//==============================================================================
void Animator::CrossFade(AnimationClip* clip, float fadeTime, int layer)
{
	if (layer < 0 || layer >= MAX_LAYERS) return;

	// 現在のアニメーションをフェードアウト用に保存
	m_FadeOutLayers[layer] = m_Layers[layer];

	// 新しいアニメーションを設定
	m_Layers[layer].clip = clip;
	m_Layers[layer].time = 0.0f;
	m_Layers[layer].weight = 1.0f;
	m_Layers[layer].speed = 1.0f;
	m_Layers[layer].loop = true;

	// フェード開始
	m_FadeTime[layer] = fadeTime;
	m_FadeElapsed[layer] = 0.0f;
	m_IsFading[layer] = true;
}

//==============================================================================
// ボーン行列取得
//==============================================================================
const std::vector<XMMATRIX>& Animator::GetBoneMatrices() const
{
	static std::vector<XMMATRIX> empty;
	if (!m_Skeleton) return empty;
	return m_Skeleton->GetSkinningMatrices();
}

//==============================================================================
// アニメーション適用
//==============================================================================
void Animator::ApplyAnimation(AnimationClip* clip, float time, float weight)
{
	if (!clip || !m_Skeleton || weight <= 0.0f) return;

	float animTime = time * clip->GetTicksPerSecond();

	for (auto& bone : m_Skeleton->GetBones())
	{
		const BoneAnimation* boneAnim = clip->GetBoneAnimation(bone.name);

		if (!boneAnim)
		{
			// アニメーションがないボーンはスキップ（初期ポーズを維持）
			continue;
		}

		Vector3 position = boneAnim->GetPosition(animTime);
		Quaternion rotation = boneAnim->GetRotation(animTime);
		Vector3 scale = boneAnim->GetScale(animTime);

		if (weight >= 1.0f)
		{
			bone.localPosition = position;
			bone.localRotation = rotation;
			bone.localScale = scale;
		}
		else
		{
			// ブレンド
			bone.localPosition = Vector3::Lerp(bone.localPosition, position, weight);
			bone.localRotation = Quaternion::Slerp(bone.localRotation, rotation, weight);
			bone.localScale = Vector3::Lerp(bone.localScale, scale, weight);
		}
	}
}

//==============================================================================
// ボーンブレンド
//==============================================================================
void Animator::BlendBones()
{
	if (!m_Skeleton) return;

	// 各レイヤーのアニメーションを適用
	bool hasAnimation = false;

	for (int i = 0; i < MAX_LAYERS; i++)
	{
		float layerWeight = m_Layers[i].weight;

		// フェード中の場合
		if (m_IsFading[i] && m_FadeTime[i] > 0.0f)
		{
			float fadeProgress = m_FadeElapsed[i] / m_FadeTime[i];

			// フェードアウト中のアニメーション
			if (m_FadeOutLayers[i].clip)
			{
				ApplyAnimation(m_FadeOutLayers[i].clip, m_FadeOutLayers[i].time,
					layerWeight * (1.0f - fadeProgress));
				hasAnimation = true;
			}

			// フェードイン中のアニメーション
			if (m_Layers[i].clip)
			{
				ApplyAnimation(m_Layers[i].clip, m_Layers[i].time,
					layerWeight * fadeProgress);
				hasAnimation = true;
			}
		}
		else
		{
			// 通常再生
			if (m_Layers[i].clip)
			{
				ApplyAnimation(m_Layers[i].clip, m_Layers[i].time, layerWeight);
				hasAnimation = true;
			}
		}
	}

	// ボーン行列を更新
	m_Skeleton->UpdateBoneMatrices();
}
