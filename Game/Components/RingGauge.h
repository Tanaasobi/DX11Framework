#pragma once
#include "Core/Object/Component.h"
#include <d3d11.h>
#include <DirectXMath.h>

class GaugeShader;

class RingGauge : public Component
{
public:
	RingGauge();
	virtual ~RingGauge();

	void Init(float radius, float innerRadiusRatio = 0.8f);
	void Update(float deltaTime) override;
	void Render() override;

	void SetProgress(float progress);
	void SetColor(float r, float g, float b, float a);
	void SetVisible(bool visible) { m_IsVisible = visible; }

private:
	struct GaugeBuffer
	{
		DirectX::XMFLOAT4 Color;
		float Progress;
		float InnerRadius;
		float Padding[2];
	};

	GaugeShader* m_Shader = nullptr;
	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_ConstantBuffer = nullptr;

	bool m_IsVisible = false;
	float m_Radius = 1.0f;

	GaugeBuffer m_BufferData;
};
