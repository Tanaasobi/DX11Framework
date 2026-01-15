#pragma once

//==============================================================================
// LightningShader.h - 雷用シェーダー
//==============================================================================

#include "Core/Graphics/Shader/IShader.h"
#include "Core/System/main.h"
#include <string>

class LightningShader : public IShader
{
public:
	LightningShader();
	virtual ~LightningShader();

	bool Load(const std::wstring& vsFile, const std::wstring& psFile);
	void Set() override;

private:
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;
};
