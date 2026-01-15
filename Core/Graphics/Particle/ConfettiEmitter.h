#pragma once

//==============================================================================
// ConfettiEmitter.h - 紙吹雪エミッター
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/System/main.h"
#include "Core/Math/Vector3.h"
#include "ParticleShader.h"
#include <vector>
#include <random>

//==============================================================================
// 紙吹雪パーティクル
//==============================================================================
struct Confetti
{
	Vector3 position;
	Vector3 velocity;
	float   rotationX;
	float   rotationY;
	float   rotationZ;
	float   rotationSpeedX;
	float   rotationSpeedY;
	float   rotationSpeedZ;
	float   size;
	float   life;
	float   lifeMax;
	float   swayPhase;      // 揺れの位相
	float   swaySpeed;      // 揺れの速さ
	float   swayAmount;     // 揺れの大きさ
	DirectX::XMFLOAT4 color;
	bool    active;
};

//==============================================================================
// GPU用頂点データ
//==============================================================================
struct ConfettiVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 TexCoord;
};

//==============================================================================
// ConfettiEmitter クラス
//==============================================================================
class ConfettiEmitter : public Component
{
public:
	ConfettiEmitter();
	virtual ~ConfettiEmitter();

	bool Init(int maxParticles);
	void Update(float deltaTime) override;
	void Render() override;

	// バースト発生
	void Burst(int count, const Vector3& position);
	// バースト発生（方向指定版）
	void Burst(int count, const Vector3& position, const Vector3& direction);

	// シェーダー・テクスチャ設定
	void SetShader(IShader* shader) { m_Shader = shader; }
	void SetTexture(ID3D11ShaderResourceView* texture) { m_Texture = texture; }

	// パラメータ
	float gravity = 3.0f;
	float lifeMin = 2.0f;
	float lifeMax = 4.0f;
	float sizeMin = 0.2f;
	float sizeMax = 0.4f;
	float spreadX = 10.0f;
	float spreadY = 2.0f;
	float spreadZ = 17.0f;
	float initialSpeedMin = 5.0f;
	float initialSpeedMax = 10.0f;

private:
	std::vector<Confetti> m_Particles;
	int m_MaxParticles = 0;

	ID3D11Buffer* m_VertexBuffer = nullptr;
	IShader* m_Shader = nullptr;
	ID3D11ShaderResourceView* m_Texture = nullptr;
	int m_VertexCount = 0;

	std::mt19937 m_Rng;

	// カラーパレット
	std::vector<DirectX::XMFLOAT4> m_Colors;

	void UpdateVertexBuffer();
	float RandomRange(float min, float max);
	DirectX::XMFLOAT4 RandomColor();
};
