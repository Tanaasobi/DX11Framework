//==============================================================================
// RimLightSkinnedShader.cpp - リムライト付きスキニングシェーダー実装
//==============================================================================

#include "RimLightSkinnedShader.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
RimLightSkinnedShader::RimLightSkinnedShader()
{
	// デフォルト値
	m_RimParams.RimColor = XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f);  // 青白い光
	m_RimParams.RimPower = 3.0f;
	m_RimParams.RimIntensity = 1.5f;
}

//==============================================================================
// デストラクタ
//==============================================================================
RimLightSkinnedShader::~RimLightSkinnedShader()
{
	SAFE_RELEASE(m_RimBuffer);
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_VertexShader);
}

//==============================================================================
// ロード
//==============================================================================
bool RimLightSkinnedShader::Load(const std::wstring& vsFile, const std::wstring& psFile)
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
			Logger::ErrorFormat("RimLightSkinnedShader VS error: %s", (char*)errorBlob->GetBufferPointer());
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
			Logger::ErrorFormat("RimLightSkinnedShader PS error: %s", (char*)errorBlob->GetBufferPointer());
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

	// リムバッファ作成
	if (!CreateRimBuffer())
	{
		return false;
	}

	Logger::Info("RimLightSkinnedShader loaded");
	return true;
}

//==============================================================================
// 入力レイアウト作成
//==============================================================================
bool RimLightSkinnedShader::CreateInputLayout(ID3DBlob* vsBlob)
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
// リムバッファ作成
//==============================================================================
bool RimLightSkinnedShader::CreateRimBuffer()
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(RimLightParams);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_RimBuffer);
	return SUCCEEDED(hr);
}

//==============================================================================
// シェーダーセット
//==============================================================================
void RimLightSkinnedShader::Set()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	context->IASetInputLayout(m_InputLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);

	// リムバッファ更新してセット（b7に設定）
	UpdateRimBuffer();
	context->PSSetConstantBuffers(7, 1, &m_RimBuffer);
}

//==============================================================================
// リムバッファ更新
//==============================================================================
void RimLightSkinnedShader::UpdateRimBuffer()
{
	Renderer::GetDeviceContext()->UpdateSubresource(m_RimBuffer, 0, nullptr, &m_RimParams, 0, 0);
}

//==============================================================================
// リムカラー設定
//==============================================================================
void RimLightSkinnedShader::SetRimColor(float r, float g, float b)
{
	m_RimParams.RimColor = DirectX::XMFLOAT4(r, g, b, 1.0f);
}

//==============================================================================
// リムパワー設定
//==============================================================================
void RimLightSkinnedShader::SetRimPower(float power)
{
	m_RimParams.RimPower = power;
}

//==============================================================================
// リム強度設定
//==============================================================================
void RimLightSkinnedShader::SetRimIntensity(float intensity)
{
	m_RimParams.RimIntensity = intensity;
}
