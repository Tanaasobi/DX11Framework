//==============================================================================
// SkinnedShader.cpp - スキニング用シェーダー実装
//==============================================================================

#include "SkinnedShader.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"

//==============================================================================
// コンストラクタ
//==============================================================================
SkinnedShader::SkinnedShader()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
SkinnedShader::~SkinnedShader()
{
}

//==============================================================================
// 入力レイアウト作成（スキニング用）
//==============================================================================
bool SkinnedShader::CreateInputLayout(ID3DBlob* vsBlob)
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT hr = Renderer::GetDevice()->CreateInputLayout(
		layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		&m_InputLayout
	);

	if (SUCCEEDED(hr))
	{
		Logger::Info("SkinnedShader loaded");
	}

	return SUCCEEDED(hr);
}
