//==============================================================================
// LightningShader.cpp - 雷用シェーダー実装
//==============================================================================

#include "LightningShader.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

LightningShader::LightningShader()
{
}

LightningShader::~LightningShader()
{
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_VertexShader);
}

bool LightningShader::Load(const std::wstring& vsFile, const std::wstring& psFile)
{
	HRESULT hr;
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// 頂点シェーダー
	hr = D3DCompileFromFile(vsFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("LightningShader VS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	// ピクセルシェーダー
	hr = D3DCompileFromFile(psFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("LightningShader PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	ID3D11Device* device = Renderer::GetDevice();

	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);

	// 入力レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);

	vsBlob->Release();
	psBlob->Release();

	if (FAILED(hr)) return false;

	Logger::Info("LightningShader loaded");
	return true;
}

void LightningShader::Set()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	context->IASetInputLayout(m_InputLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);
}
