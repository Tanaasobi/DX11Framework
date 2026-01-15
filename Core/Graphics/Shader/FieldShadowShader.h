#pragma once

//==============================================================================
// FieldShadowShader.h - フィールド用影付きシェーダー
//==============================================================================

#include "IShader.h"
#include "Core/System/main.h"
#include <string>

class FieldShadowShader : public IShader
{
public:
	FieldShadowShader();
	virtual ~FieldShadowShader();

	bool Load(const std::wstring& vsFile, const std::wstring& psFile);
	void Set() override;

private:
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;

	bool CreateInputLayout(ID3DBlob* vsBlob);
};
