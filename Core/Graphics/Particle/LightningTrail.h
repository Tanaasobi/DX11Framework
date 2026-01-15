#pragma once

//==============================================================================
// LightningTrail.h - 雷トレイルシステム
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/System/main.h"
#include "Core/Math/Vector3.h"
#include "Core/Graphics/Shader/IShader.h"
#include <vector>
#include <random>

//==============================================================================
// 雷セグメント
//==============================================================================
struct LightningSegment
{
	Vector3 start;
	Vector3 end;
	float   life;
	float   lifeMax;
	float   width;
	float   alpha;
};

//==============================================================================
// 雷頂点（GPU用）
//==============================================================================
struct LightningVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 TexCoord;
};

//==============================================================================
// LightningTrail クラス
//==============================================================================
class LightningTrail : public Component
{
public:
	LightningTrail();
	virtual ~LightningTrail();

	bool Init(int maxSegments);
	void Update(float deltaTime) override;
	void Render() override;

	// シェーダー設定
	void SetShader(IShader* shader) { m_Shader = shader; }

	// テクスチャ設定
	void SetTexture(ID3D11ShaderResourceView* texture) { m_Texture = texture; }

	// 発生制御
	void Play() { m_IsPlaying = true; }
	void Stop() { m_IsPlaying = false; m_HasLastPosition = false; }
	bool IsPlaying() const { return m_IsPlaying; }

	// パラメータ
	float segmentLife = 0.15f;
	float width = 0.15f;
	float jitter = 0.3f;
	int   subdivisions = 4;
	float emitDistance = 0.1f;
	DirectX::XMFLOAT4 colorCore = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT4 colorEdge = { 0.3f, 0.5f, 1.0f, 0.5f };

private:
	std::vector<LightningSegment> m_Segments;
	int m_MaxSegments = 100;

	Vector3 m_LastPosition;
	bool m_HasLastPosition = false;
	bool m_IsPlaying = false;

	ID3D11Buffer* m_VertexBuffer = nullptr;
	IShader* m_Shader = nullptr;
	ID3D11ShaderResourceView* m_Texture = nullptr;
	int m_VertexCount = 0;

	std::mt19937 m_Rng;

	void EmitAuto(const Vector3& currentPos);
	void Emit(const Vector3& from, const Vector3& to);
	void GenerateLightningPath(const Vector3& from, const Vector3& to, std::vector<Vector3>& outPoints);
	void UpdateVertexBuffer();
	float RandomRange(float min, float max);
};
