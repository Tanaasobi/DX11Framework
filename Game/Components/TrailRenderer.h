#pragma once
#include "Core/Object/Component.h"
#include <deque>
#include <vector>
#include <DirectXMath.h>
#include <d3d11.h> // ID3D11Buffer等のために必要

// 頂点構造体
struct TrailVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT2 TexCoord;
};

// 軌跡のデータポイント
struct TrailPoint
{
	DirectX::XMFLOAT3 Position;
	float Time; // 生成された時刻
};

class TrailRenderer : public Component
{
public:
	TrailRenderer();
	virtual ~TrailRenderer();

	// 初期化 (テクスチャパス, 幅, 寿命)
	void Init(const std::string& texturePath, float width = 1.0f, float lifetime = 0.5f);

	void Update(float deltaTime) override;
	void Render() override;

	// 色の設定
	void SetColor(const DirectX::XMFLOAT4& startColor, const DirectX::XMFLOAT4& endColor);
	// 幅の設定
	void SetWidth(float width) { m_Width = width; }

private:
	// 内部シェーダークラス
	class TrailShader* m_Shader = nullptr;
	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11ShaderResourceView* m_TextureSRV = nullptr;

	std::deque<TrailPoint> m_Points;

	float m_Width = 1.0f;
	float m_Lifetime = 0.5f;
	float m_MinVertexDistance = 0.1f; // この距離以上動いたらポイントを追加

	DirectX::XMFLOAT4 m_StartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT4 m_EndColor = { 1.0f, 1.0f, 1.0f, 0.0f };

	void UpdateVertexBuffer();
	void CreateVertexBuffer();
};
