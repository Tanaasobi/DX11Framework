#pragma once

//==============================================================================
// RimLightSkinnedShader.h - リムライト付きスキニングシェーダー
//==============================================================================

#include "IShader.h"
#include "Core/System/main.h"
#include <string>

class RimLightSkinnedShader : public IShader
{
public:
	RimLightSkinnedShader();
	virtual ~RimLightSkinnedShader();

	bool Load(const std::wstring& vsFile, const std::wstring& psFile);
	void Set() override;

	// リムライトパラメータ設定
	void SetRimColor(float r, float g, float b);
	void SetRimPower(float power);
	void SetRimIntensity(float intensity);
	void UpdateRimBuffer();

private:
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;
	ID3D11Buffer* m_RimBuffer = nullptr;

	struct RimLightParams
	{
		DirectX::XMFLOAT4 RimColor;     // リムの色
		float RimPower;                  // リムの鋭さ（大きいほど縁だけ光る）
		float RimIntensity;              // リムの強さ
		float Padding[2];
	};

	RimLightParams m_RimParams;

	bool CreateInputLayout(ID3DBlob* vsBlob);
	bool CreateRimBuffer();
};
