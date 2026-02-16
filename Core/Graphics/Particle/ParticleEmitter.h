#pragma once

//==============================================================================
// ParticleEmitter.h - パーティクルエミッターコンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "ParticleSystem.h"
#include "ParticleRenderer.h"
#include "ParticleShader.h"
#include <random>

class ParticleEmitter : public Component
{
public:
	ParticleEmitter();
	virtual ~ParticleEmitter();

	bool Init(int maxParticles);
	void Update(float deltaTime) override;
	void Render() override;

	// 設定
	ParticleSettings settings;

	// テクスチャ設定
	void SetTexture(const std::string& fileName);
	void SetTexture(ID3D11ShaderResourceView* texture);

	// シェーダー設定
	void SetShader(ParticleShader* shader);

	// 発生制御
	void Play() { m_IsPlaying = true; }
	void Stop() { m_IsPlaying = false; }
	void Burst(int count);  // 一度に大量発生
	void Burst(int count, const Vector3& position, const Vector3& direction, float speed, float spread = 0.5f);

private:
	ParticlePool     m_Pool;
	ParticleRenderer m_Renderer;
	ParticleShader* m_Shader = nullptr;

	ID3D11ShaderResourceView* m_Texture = nullptr;

	bool  m_IsPlaying = true;
	float m_EmitAccumulator = 0.0f;

	std::mt19937 m_Rng;

	void EmitParticle();
	float RandomRange(float min, float max);
};
