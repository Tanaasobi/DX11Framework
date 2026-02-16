#pragma once

//==============================================================================
// ParticleSystem.h - パーティクルシステム
//==============================================================================

#include "Core/System/main.h"
#include "Core/Math/Vector3.h"
#include <vector>

//==============================================================================
// パーティクル1個のデータ
//==============================================================================
struct Particle
{
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	float   rotation;
	float   rotationSpeed;
	float   size;
	float   sizeStart;
	float   sizeEnd;
	float   life;
	float   lifeMax;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 colorStart;
	DirectX::XMFLOAT4 colorEnd;
	bool    active;
};

//==============================================================================
// GPUに送るインスタンスデータ（1パーティクルあたり）
//==============================================================================
struct ParticleInstance
{
	DirectX::XMFLOAT3 Position;
	float             Size;
	DirectX::XMFLOAT4 Color;
	float             Rotation;
	DirectX::XMFLOAT3 Velocity;
};

//==============================================================================
// パーティクル設定
//==============================================================================
struct ParticleSettings
{
	// 発生
	int   maxParticles = 1000;
	float emitRate = 50.0f;          // 秒間発生数

	// 寿命
	float lifeMin = 0.5f;
	float lifeMax = 2.0f;

	// サイズ
	float sizeStart = 0.2f;
	float sizeEnd = 0.0f;

	// 速度
	Vector3 velocityMin = Vector3(-1, 1, -1);
	Vector3 velocityMax = Vector3(1, 3, 1);

	// 加速度（重力など）
	Vector3 acceleration = Vector3(0, -2, 0);

	// 回転
	float rotationMin = 0.0f;
	float rotationMax = 360.0f;
	float rotationSpeedMin = -180.0f;
	float rotationSpeedMax = 180.0f;

	// 色
	DirectX::XMFLOAT4 colorStart = { 1, 1, 1, 1 };
	DirectX::XMFLOAT4 colorEnd = { 1, 1, 1, 0 };

	// ブレンドモード
	bool additiveBlend = true;

	//　垂直方向への拡散量（0で完全に水平、1で全方向）
	float spread = 0.5f;
};

//==============================================================================
// ParticlePool クラス - パーティクルプール管理
//==============================================================================
class ParticlePool
{
public:
	ParticlePool();
	~ParticlePool();

	void Init(int maxParticles);
	void Uninit();

	// パーティクル発生
	Particle* Emit();

	// 更新
	void Update(float deltaTime);

	// アクティブなパーティクル取得
	const std::vector<Particle>& GetParticles() const { return m_Particles; }
	int GetActiveCount() const { return m_ActiveCount; }

private:
	std::vector<Particle> m_Particles;
	int m_ActiveCount = 0;
	int m_NextIndex = 0;
};
