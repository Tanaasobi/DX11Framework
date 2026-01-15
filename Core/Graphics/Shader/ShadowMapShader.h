#pragma once

//==============================================================================
// ShadowMapShader.h - シャドウマップ用シェーダー
//==============================================================================

#include "IShader.h"
#include "Core/System/main.h"
#include <string>

class ShadowMapShader : public IShader
{
public:
	ShadowMapShader();
	virtual ~ShadowMapShader();

	bool Load(const std::wstring& vsFile, const std::wstring& psFile);
	void Set() override;

private:
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;

	bool CreateInputLayout(ID3DBlob* vsBlob);
};
