//==============================================================================
// ParticleShader.cpp - パーティクル用シェーダー実装
//==============================================================================

#include "ParticleShader.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

ParticleShader::ParticleShader()
{
}

ParticleShader::~ParticleShader()
{
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_VertexShader);
}

bool ParticleShader::Load(const std::wstring& vsFile, const std::wstring& psFile)
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
			Logger::ErrorFormat("ParticleShader VS error: %s", (char*)errorBlob->GetBufferPointer());
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
			Logger::ErrorFormat("ParticleShader PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	ID3D11Device* device = Renderer::GetDevice();

	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);

	// 入力レイアウト（頂点 + インスタンス）
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		// 頂点データ（slot 0）
		{ "POSITION",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
		{ "TEXCOORD",      0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },

		// インスタンスデータ（slot 1）
		{ "INST_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INST_SIZE",     0, DXGI_FORMAT_R32_FLOAT,       1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INST_COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INST_ROTATION", 0, DXGI_FORMAT_R32_FLOAT,       1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INST_VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 36, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);

	vsBlob->Release();
	psBlob->Release();

	if (FAILED(hr)) return false;

	Logger::Info("ParticleShader loaded");
	return true;
}

void ParticleShader::Set()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	context->IASetInputLayout(m_InputLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);
}
