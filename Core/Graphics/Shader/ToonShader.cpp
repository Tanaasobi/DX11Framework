//==============================================================================
// ToonShader.cpp - トゥーンシェーダー実装
//==============================================================================

#include "ToonShader.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
ToonShader::ToonShader()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
ToonShader::~ToonShader()
{
	SAFE_RELEASE(m_OutlineBuffer);
	SAFE_RELEASE(m_ToonBuffer);
	SAFE_RELEASE(m_OutlineInputLayout);
	SAFE_RELEASE(m_OutlinePixelShader);
	SAFE_RELEASE(m_OutlineVertexShader);
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_VertexShader);
}

//==============================================================================
// 定数バッファ作成
//==============================================================================
bool ToonShader::CreateConstantBuffers()
{
	ID3D11Device* device = Renderer::GetDevice();
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	// トゥーンバッファ
	bufferDesc.ByteWidth = sizeof(TOON_SETTINGS);
	hr = device->CreateBuffer(&bufferDesc, nullptr, &m_ToonBuffer);
	if (FAILED(hr)) return false;

	// アウトラインバッファ
	bufferDesc.ByteWidth = sizeof(OUTLINE_SETTINGS);
	hr = device->CreateBuffer(&bufferDesc, nullptr, &m_OutlineBuffer);
	if (FAILED(hr)) return false;

	// デフォルト設定
	TOON_SETTINGS toon = {};
	toon.Levels = 3;
	toon.Edge = 1.0f;
	toon.RimPower = 3.0f;
	toon.RimIntensity = 0.5f;
	toon.RimColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetToonSettings(toon);

	OUTLINE_SETTINGS outline = {};
	outline.Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	outline.Width = 0.01f;
	SetOutlineSettings(outline);

	return true;
}

//==============================================================================
// 入力レイアウト作成
//==============================================================================
bool ToonShader::CreateInputLayout(ID3DBlob* vsBlob, bool skinned)
{
	ID3D11Device* device = Renderer::GetDevice();
	HRESULT hr;

	if (skinned)
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

		hr = device->CreateInputLayout(layout, ARRAYSIZE(layout),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
	}
	else
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		hr = device->CreateInputLayout(layout, ARRAYSIZE(layout),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
	}

	return SUCCEEDED(hr);
}

//==============================================================================
// 通常モデル用シェーダー読み込み
//==============================================================================
bool ToonShader::Load(const std::wstring& vsFile, const std::wstring& psFile)
{
	m_IsSkinned = false;

	HRESULT hr;
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	hr = D3DCompileFromFile(vsFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("ToonShader VS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	hr = D3DCompileFromFile(psFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("ToonShader PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	ID3D11Device* device = Renderer::GetDevice();
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);

	if (!CreateInputLayout(vsBlob, false))
	{
		vsBlob->Release();
		psBlob->Release();
		return false;
	}

	vsBlob->Release();
	psBlob->Release();

	if (!CreateConstantBuffers()) return false;

	Logger::Info("ToonShader loaded");
	return true;
}

//==============================================================================
// スキニングモデル用シェーダー読み込み
//==============================================================================
bool ToonShader::LoadSkinned(const std::wstring& vsFile, const std::wstring& psFile)
{
	m_IsSkinned = true;

	HRESULT hr;
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	hr = D3DCompileFromFile(vsFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("ToonShader VS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	hr = D3DCompileFromFile(psFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("ToonShader PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	ID3D11Device* device = Renderer::GetDevice();
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);

	if (!CreateInputLayout(vsBlob, true))
	{
		vsBlob->Release();
		psBlob->Release();
		return false;
	}

	vsBlob->Release();
	psBlob->Release();

	if (!CreateConstantBuffers()) return false;

	Logger::Info("ToonShader (Skinned) loaded");
	return true;
}

//==============================================================================
// アウトライン用シェーダー読み込み
//==============================================================================
bool ToonShader::LoadOutline(const std::wstring& vsFile, const std::wstring& psFile)
{
	HRESULT hr;
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	hr = D3DCompileFromFile(vsFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("Outline VS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	hr = D3DCompileFromFile(psFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("Outline PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	ID3D11Device* device = Renderer::GetDevice();
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_OutlineVertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_OutlinePixelShader);

	// 入力レイアウト（通常モデル用）
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_OutlineInputLayout);

	vsBlob->Release();
	psBlob->Release();

	Logger::Info("Outline shader loaded");
	return SUCCEEDED(hr);
}

//==============================================================================
// スキニング対応アウトライン用シェーダー読み込み
//==============================================================================
bool ToonShader::LoadSkinnedOutline(const std::wstring& vsFile, const std::wstring& psFile)
{
	HRESULT hr;
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	hr = D3DCompileFromFile(vsFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("Skinned Outline VS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	hr = D3DCompileFromFile(psFile.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &psBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("Skinned Outline PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	ID3D11Device* device = Renderer::GetDevice();
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_OutlineVertexShader);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_OutlinePixelShader);

	// 入力レイアウト（スキニング用）
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",        0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_OutlineInputLayout);

	vsBlob->Release();
	psBlob->Release();

	Logger::Info("Skinned Outline shader loaded");
	return SUCCEEDED(hr);
}

//==============================================================================
// メインシェーダーをセット
//==============================================================================
void ToonShader::Set()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	context->IASetInputLayout(m_InputLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);
	context->PSSetConstantBuffers(7, 1, &m_ToonBuffer);
}

//==============================================================================
// アウトラインシェーダーをセット
//==============================================================================
void ToonShader::SetOutline()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	context->IASetInputLayout(m_OutlineInputLayout);
	context->VSSetShader(m_OutlineVertexShader, nullptr, 0);
	context->PSSetShader(m_OutlinePixelShader, nullptr, 0);
	context->VSSetConstantBuffers(7, 1, &m_OutlineBuffer);
}

//==============================================================================
// トゥーン設定
//==============================================================================
void ToonShader::SetToonSettings(const TOON_SETTINGS& settings)
{
	Renderer::GetDeviceContext()->UpdateSubresource(m_ToonBuffer, 0, nullptr, &settings, 0, 0);
}

//==============================================================================
// アウトライン設定
//==============================================================================
void ToonShader::SetOutlineSettings(const OUTLINE_SETTINGS& settings)
{
	Renderer::GetDeviceContext()->UpdateSubresource(m_OutlineBuffer, 0, nullptr, &settings, 0, 0);
}
