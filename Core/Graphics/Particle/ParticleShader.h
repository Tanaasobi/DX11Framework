#pragma once

//==============================================================================
// ParticleShader.h - パーティクル用シェーダー
//==============================================================================

#include "Core/Graphics/Shader/IShader.h"
#include "Core/System/main.h"
#include <string>

class ParticleShader : public IShader
{
public:
	ParticleShader();
	virtual ~ParticleShader();

	bool Load(const std::wstring& vsFile, const std::wstring& psFile);
	void Set() override;

private:
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;
};
