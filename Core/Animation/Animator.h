#pragma once
//==============================================================================
// Animator.h - アニメーターコンポーネント
//==============================================================================
#include "Core/Object/Component.h"
#include "Skeleton.h"
#include "AnimationClip.h"
#include <memory>

//==============================================================================
// アニメーションレイヤー（ブレンド用）
//==============================================================================
struct AnimationLayer
{
	AnimationClip* clip = nullptr;
	float          time = 0.0f;
	float          weight = 1.0f;
	float          speed = 1.0f;
	bool           loop = true;
};

//==============================================================================
// Animator クラス
//==============================================================================
class Animator : public Component
{
public:
	Animator();
	virtual ~Animator();

	void Update(float deltaTime) override;

	// スケルトン設定
	void SetSkeleton(std::shared_ptr<Skeleton> skeleton);
	Skeleton* GetSkeleton() const { return m_Skeleton.get(); }

	// アニメーション再生
	void Play(AnimationClip* clip, int layer = 0, float fadeTime = 0.0f);
	void Stop(int layer = 0);

	// 1回だけ再生（最大時間指定可能）
	void PlayOnce(AnimationClip* clip, float blendTime = 0.1f, float maxDuration = 0.0f);

	// レイヤー設定
	void SetLayerWeight(int layer, float weight);
	void SetLayerSpeed(int layer, float speed);
	void SetLooping(int layer, bool loop);

	// 再生状態
	float GetTime(int layer = 0) const;
	float GetNormalizedTime(int layer = 0) const;
	bool IsPlaying(int layer = 0) const;
	bool IsPlaying(AnimationClip* clip) const;

	// クロスフェード
	void CrossFade(AnimationClip* clip, float fadeTime, int layer = 0);

	// ボーン行列取得
	const std::vector<DirectX::XMMATRIX>& GetBoneMatrices() const;

private:
	static constexpr int MAX_LAYERS = 4;

	std::shared_ptr<Skeleton> m_Skeleton;
	AnimationLayer m_Layers[MAX_LAYERS];

	// クロスフェード用
	AnimationLayer m_FadeOutLayers[MAX_LAYERS];
	float m_FadeTime[MAX_LAYERS] = {};
	float m_FadeElapsed[MAX_LAYERS] = {};
	bool  m_IsFading[MAX_LAYERS] = {};

	// PlayOnce用（元のアニメーションに戻るため）
	AnimationClip* m_ReturnClip[MAX_LAYERS] = {};
	bool m_WaitingReturn[MAX_LAYERS] = {};

	void ApplyAnimation(AnimationClip* clip, float time, float weight);
	void BlendBones();
};
