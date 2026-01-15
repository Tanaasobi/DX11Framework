//==============================================================================
// ShadowMapShader.cpp - シャドウマップ用シェーダー実装
//==============================================================================

#include "ShadowMapShader.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

//==============================================================================
// コンストラクタ
//==============================================================================
ShadowMapShader::ShadowMapShader()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
ShadowMapShader::~ShadowMapShader()
{
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_VertexShader);
}

//==============================================================================
// ロード
//==============================================================================
bool ShadowMapShader::Load(const std::wstring& vsFile, const std::wstring& psFile)
{
	HRESULT hr;
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// 頂点シェーダーコンパイル
	hr = D3DCompileFromFile(vsFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("ShadowMapShader VS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	// ピクセルシェーダーコンパイル
	hr = D3DCompileFromFile(psFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("ShadowMapShader PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	ID3D11Device* device = Renderer::GetDevice();

	// シェーダー作成
	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		nullptr, &m_VertexShader);
	if (FAILED(hr))
	{
		vsBlob->Release();
		psBlob->Release();
		return false;
	}

	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
		nullptr, &m_PixelShader);
	if (FAILED(hr))
	{
		vsBlob->Release();
		psBlob->Release();
		return false;
	}

	// 入力レイアウト作成
	if (!CreateInputLayout(vsBlob))
	{
		vsBlob->Release();
		psBlob->Release();
		return false;
	}

	vsBlob->Release();
	psBlob->Release();

	Logger::Info("ShadowMapShader loaded");
	return true;
}

//==============================================================================
// 入力レイアウト作成（スキニング対応）
//==============================================================================
bool ShadowMapShader::CreateInputLayout(ID3DBlob* vsBlob)
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

	return SUCCEEDED(hr);
}

//==============================================================================
// シェーダーセット
//==============================================================================
void ShadowMapShader::Set()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	context->IASetInputLayout(m_InputLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);
}
