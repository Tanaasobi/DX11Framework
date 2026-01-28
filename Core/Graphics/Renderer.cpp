//==============================================================================
// Renderer.cpp - DirectX11 レンダリング管理クラス実装
//==============================================================================

#include "Renderer.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

//==============================================================================
// 静的メンバ変数の定義
//==============================================================================

// デバイス関連
ID3D11Device* Renderer::m_Device = nullptr;
ID3D11DeviceContext* Renderer::m_DeviceContext = nullptr;
IDXGISwapChain* Renderer::m_SwapChain = nullptr;

// レンダーターゲット
ID3D11RenderTargetView* Renderer::m_RenderTargetView = nullptr;
ID3D11DepthStencilView* Renderer::m_DepthStencilView = nullptr;

// 定数バッファ
ID3D11Buffer* Renderer::m_WorldBuffer = nullptr;
ID3D11Buffer* Renderer::m_ViewBuffer = nullptr;
ID3D11Buffer* Renderer::m_ProjectionBuffer = nullptr;
ID3D11Buffer* Renderer::m_BoneBuffer = nullptr;
ID3D11Buffer* Renderer::m_MaterialBuffer = nullptr;
ID3D11Buffer* Renderer::m_LightBuffer = nullptr;
ID3D11Buffer* Renderer::m_CameraBuffer = nullptr;

// 深度ステンシルステート
ID3D11DepthStencilState* Renderer::m_DepthStateEnable = nullptr;
ID3D11DepthStencilState* Renderer::m_DepthStateDisable = nullptr;
ID3D11DepthStencilState* Renderer::m_DepthStateEnableWriteDisable = nullptr;

// ブレンドステート
ID3D11BlendState* Renderer::m_BlendStateNone = nullptr;
ID3D11BlendState* Renderer::m_BlendStateAlpha = nullptr;
ID3D11BlendState* Renderer::m_BlendStateAdd = nullptr;

// ラスタライザステート
ID3D11RasterizerState* Renderer::m_RasterizerCullBack = nullptr;
ID3D11RasterizerState* Renderer::m_RasterizerCullFront = nullptr;
ID3D11RasterizerState* Renderer::m_RasterizerCullNone = nullptr;

// サンプラーステート
ID3D11SamplerState* Renderer::m_SamplerState = nullptr;

// オフスクリーン（シーン描画先）
ID3D11Texture2D* Renderer::m_SceneColorTex = nullptr;
ID3D11RenderTargetView* Renderer::m_SceneColorRTV = nullptr;
ID3D11ShaderResourceView* Renderer::m_SceneColorSRV = nullptr;

ID3D11Texture2D* Renderer::m_SceneDepthTex = nullptr;
ID3D11DepthStencilView* Renderer::m_SceneDepthDSV = nullptr;

// エッジ抽出シェーダ
ID3D11VertexShader* Renderer::m_EdgeVS = nullptr;
ID3D11PixelShader* Renderer::m_EdgePS = nullptr;
ID3D11Buffer* Renderer::m_EdgeParamBuffer = nullptr;

// clampサンプラ
ID3D11SamplerState* Renderer::m_SamplerClamp = nullptr;

//==============================================================================
// エッジ用定数バッファ
//==============================================================================
struct EdgeParamsCB
{
	DirectX::XMFLOAT2 TexelSize;
	float Threshold;
	float EdgePower;
	DirectX::XMFLOAT4 EdgeColor;
};

//==============================================================================
// 初期化
//==============================================================================
bool Renderer::Init(HWND hWnd)
{
	HRESULT hr;

	//--------------------------------------------------------------------------
	// デバイスとスワップチェインの作成
	//--------------------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	swapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL featureLevelOut;

	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		featureLevels,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_SwapChain,
		&m_Device,
		&featureLevelOut,
		&m_DeviceContext
	);

	if (FAILED(hr))
	{
		MessageBox(hWnd, L"D3D11CreateDeviceAndSwapChain failed", L"Error", MB_OK);
		return false;
	}

	//--------------------------------------------------------------------------
	// レンダーターゲットビューの作成
	//--------------------------------------------------------------------------
	ID3D11Texture2D* backBuffer = nullptr;
	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"GetBuffer failed", L"Error", MB_OK);
		return false;
	}

	hr = m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_RenderTargetView);
	backBuffer->Release();
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"CreateRenderTargetView failed", L"Error", MB_OK);
		return false;
	}

	//--------------------------------------------------------------------------
	// 深度ステンシルバッファとビューの作成
	//--------------------------------------------------------------------------
	ID3D11Texture2D* depthStencilTexture = nullptr;
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = SCREEN_WIDTH;
	depthStencilDesc.Height = SCREEN_HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = m_Device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilTexture);
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"CreateTexture2D (DepthStencil) failed", L"Error", MB_OK);
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = m_Device->CreateDepthStencilView(depthStencilTexture, &dsvDesc, &m_DepthStencilView);
	depthStencilTexture->Release();
	if (FAILED(hr))
	{
		MessageBox(hWnd, L"CreateDepthStencilView failed", L"Error", MB_OK);
		return false;
	}

	// レンダーターゲットを設定
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	//--------------------------------------------------------------------------
	// ビューポートの設定
	//--------------------------------------------------------------------------
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_DeviceContext->RSSetViewports(1, &viewport);

	//--------------------------------------------------------------------------
	// 各種ステートの作成
	//--------------------------------------------------------------------------
	if (!CreateDepthStencilStates()) return false;
	if (!CreateBlendStates())        return false;
	if (!CreateRasterizerStates())   return false;
	if (!CreateConstantBuffers())    return false;
	if (!CreateSamplerState())       return false;

	// ポストエフェクト（エッジ抽出）
	if (!CreateSceneRenderTargets()) return false;
	if (!CreateEdgePostProcess())    return false;


	// デフォルトステートを設定
	SetDepthEnable(true);
	SetBlendMode(BlendMode::Alpha);
	SetCullingMode(true);

	return true;
}

//==============================================================================
// 終了処理
//==============================================================================
void Renderer::Uninit()
{
	// 定数バッファ
	SAFE_RELEASE(m_WorldBuffer);
	SAFE_RELEASE(m_ViewBuffer);
	SAFE_RELEASE(m_ProjectionBuffer);
	SAFE_RELEASE(m_BoneBuffer);
	SAFE_RELEASE(m_MaterialBuffer);
	SAFE_RELEASE(m_LightBuffer);
	SAFE_RELEASE(m_CameraBuffer);

	// ステート
	SAFE_RELEASE(m_DepthStateEnableWriteDisable);
	SAFE_RELEASE(m_DepthStateEnable);
	SAFE_RELEASE(m_DepthStateDisable);
	SAFE_RELEASE(m_BlendStateNone);
	SAFE_RELEASE(m_BlendStateAlpha);
	SAFE_RELEASE(m_BlendStateAdd);
	SAFE_RELEASE(m_RasterizerCullBack);
	SAFE_RELEASE(m_RasterizerCullFront);
	SAFE_RELEASE(m_RasterizerCullNone);
	SAFE_RELEASE(m_SamplerState);

	// レンダーターゲット
	SAFE_RELEASE(m_DepthStencilView);
	SAFE_RELEASE(m_RenderTargetView);

	//======================================================================
	// ポストエフェクト：エッジ抽出
	//======================================================================
	SAFE_RELEASE(m_SamplerClamp);
	SAFE_RELEASE(m_EdgeParamBuffer);
	SAFE_RELEASE(m_EdgeVS);
	SAFE_RELEASE(m_EdgePS);

	SAFE_RELEASE(m_SceneDepthDSV);
	SAFE_RELEASE(m_SceneDepthTex);
	SAFE_RELEASE(m_SceneColorSRV);
	SAFE_RELEASE(m_SceneColorRTV);
	SAFE_RELEASE(m_SceneColorTex);

	// デバイス関連
	SAFE_RELEASE(m_SwapChain);
	SAFE_RELEASE(m_DeviceContext);
	SAFE_RELEASE(m_Device);
}

//==============================================================================
// 描画開始
//==============================================================================
void Renderer::Begin()
{
	// バッファクリア（コーンフラワーブルー風の色）
	float clearColor[4] = { 0.39f, 0.58f, 0.93f, 1.0f };

	//======================================================================
	// まずはオフスクリーンにシーンを描く
	//======================================================================
	m_DeviceContext->OMSetRenderTargets(1, &m_SceneColorRTV, m_SceneDepthDSV);
	m_DeviceContext->ClearRenderTargetView(m_SceneColorRTV, clearColor);
	m_DeviceContext->ClearDepthStencilView(
		m_SceneDepthDSV,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0
	);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(SCREEN_WIDTH);
	viewport.Height = static_cast<float>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_DeviceContext->RSSetViewports(1, &viewport);
}

//==============================================================================
// 描画終了
//==============================================================================
void Renderer::End()
{
	//======================================================================
	// シーン（オフスクリーン） → エッジ抽出 → BackBuffer
	//======================================================================

	// BackBuffer へ
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	// ポストエフェクトは深度不要
	SetDepthEnable(false);
	SetBlendMode(BlendMode::None);

	// シェーダ設定
	m_DeviceContext->VSSetShader(m_EdgeVS, nullptr, 0);
	m_DeviceContext->PSSetShader(m_EdgePS, nullptr, 0);

	// Fullscreen triangle（SV_VertexIDで生成するのでVB/IL不要）
	m_DeviceContext->IASetInputLayout(nullptr);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// テクスチャ
	m_DeviceContext->PSSetShaderResources(0, 1, &m_SceneColorSRV);
	m_DeviceContext->PSSetSamplers(0, 1, &m_SamplerClamp);

	// 定数（調整ポイント）
	EdgeParamsCB cb = {};
	cb.TexelSize = DirectX::XMFLOAT2(1.0f / SCREEN_WIDTH, 1.0f / SCREEN_HEIGHT);
	cb.Threshold = 0.08f;
	cb.EdgePower = 6.0f;
	cb.EdgeColor = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	m_DeviceContext->UpdateSubresource(m_EdgeParamBuffer, 0, nullptr, &cb, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(0, 1, &m_EdgeParamBuffer);

	// 描画
	m_DeviceContext->Draw(3, 0);

	// SRVバインド解除（D3D11の警告回避）
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	m_DeviceContext->PSSetShaderResources(0, 1, nullSRV);

	// 次フレーム用に通常状態へ戻す
	SetDepthEnable(true);
	SetBlendMode(BlendMode::Alpha);

	// VSync有効でPresent
	m_SwapChain->Present(1, 0);
}

//==============================================================================
// 内部ヘルパー：オフスクリーン(RenderTarget)作成
//==============================================================================
bool Renderer::CreateSceneRenderTargets()
{
	HRESULT hr;

	//------------------------------
	// Color（RTV + SRV）
	//------------------------------
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = SCREEN_WIDTH;
	td.Height = SCREEN_HEIGHT;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	hr = m_Device->CreateTexture2D(&td, nullptr, &m_SceneColorTex);
	if (FAILED(hr)) return false;

	hr = m_Device->CreateRenderTargetView(m_SceneColorTex, nullptr, &m_SceneColorRTV);
	if (FAILED(hr)) return false;

	hr = m_Device->CreateShaderResourceView(m_SceneColorTex, nullptr, &m_SceneColorSRV);
	if (FAILED(hr)) return false;

	//------------------------------
	// Depth（DSV）
	//------------------------------
	D3D11_TEXTURE2D_DESC dd = {};
	dd.Width = SCREEN_WIDTH;
	dd.Height = SCREEN_HEIGHT;
	dd.MipLevels = 1;
	dd.ArraySize = 1;
	dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dd.SampleDesc.Count = 1;
	dd.SampleDesc.Quality = 0;
	dd.Usage = D3D11_USAGE_DEFAULT;
	dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = m_Device->CreateTexture2D(&dd, nullptr, &m_SceneDepthTex);
	if (FAILED(hr)) return false;

	hr = m_Device->CreateDepthStencilView(m_SceneDepthTex, nullptr, &m_SceneDepthDSV);
	if (FAILED(hr)) return false;

	return true;
}

//==============================================================================
// 内部ヘルパー：エッジ抽出用リソース作成
//==============================================================================
bool Renderer::CreateEdgePostProcess()
{
	HRESULT hr;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errBlob = nullptr;

	// VS
	hr = D3DCompileFromFile(
		L"Shader/PostEdgeVertexShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		0,
		0,
		&vsBlob,
		&errBlob
	);
	if (FAILED(hr))
	{
		if (errBlob) errBlob->Release();
		return false;
	}

	// PS
	hr = D3DCompileFromFile(
		L"Shader/PostEdgePixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		0,
		0,
		&psBlob,
		&errBlob
	);
	if (FAILED(hr))
	{
		SAFE_RELEASE(vsBlob);
		if (errBlob) errBlob->Release();
		return false;
	}

	hr = m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_EdgeVS);
	if (FAILED(hr))
	{
		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(psBlob);
		return false;
	}

	hr = m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_EdgePS);
	if (FAILED(hr))
	{
		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(psBlob);
		return false;
	}

	SAFE_RELEASE(vsBlob);
	SAFE_RELEASE(psBlob);

	// 定数バッファ
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = sizeof(EdgeParamsCB);
	hr = m_Device->CreateBuffer(&bd, nullptr, &m_EdgeParamBuffer);
	if (FAILED(hr)) return false;

	// clamp sampler
	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_Device->CreateSamplerState(&sd, &m_SamplerClamp);
	if (FAILED(hr)) return false;

	return true;
}

//==============================================================================
// 定数バッファ更新
//==============================================================================
void Renderer::SetWorldMatrix(const DirectX::XMMATRIX& worldMatrix)
{
	DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(worldMatrix);
	m_DeviceContext->UpdateSubresource(m_WorldBuffer, 0, nullptr, &transposed, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(0, 1, &m_WorldBuffer);
}

void Renderer::SetViewMatrix(const DirectX::XMMATRIX& viewMatrix)
{
	DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(viewMatrix);
	m_DeviceContext->UpdateSubresource(m_ViewBuffer, 0, nullptr, &transposed, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(1, 1, &m_ViewBuffer);
}

void Renderer::SetProjectionMatrix(const DirectX::XMMATRIX& projectionMatrix)
{
	DirectX::XMMATRIX transposed = DirectX::XMMatrixTranspose(projectionMatrix);
	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer, 0, nullptr, &transposed, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(2, 1, &m_ProjectionBuffer);
}

void Renderer::SetMaterial(const MATERIAL& material)
{
	m_DeviceContext->UpdateSubresource(m_MaterialBuffer, 0, nullptr, &material, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(3, 1, &m_MaterialBuffer);
	m_DeviceContext->PSSetConstantBuffers(3, 1, &m_MaterialBuffer);
}

void Renderer::SetLight(const LIGHT& light)
{
	m_DeviceContext->UpdateSubresource(m_LightBuffer, 0, nullptr, &light, 0, 0);
	m_DeviceContext->VSSetConstantBuffers(5, 1, &m_LightBuffer);
	m_DeviceContext->PSSetConstantBuffers(5, 1, &m_LightBuffer);
}

void Renderer::SetCameraPosition(const DirectX::XMFLOAT3& position)
{
	DirectX::XMFLOAT4 pos4(position.x, position.y, position.z, 1.0f);
	m_DeviceContext->UpdateSubresource(m_CameraBuffer, 0, nullptr, &pos4, 0, 0);
	m_DeviceContext->PSSetConstantBuffers(6, 1, &m_CameraBuffer);
}

//==============================================================================
// ボーン行列を設定
//==============================================================================
void Renderer::SetBoneMatrices(const DirectX::XMMATRIX* matrices, int count)
{
	if (!matrices || count <= 0) return;

	// 最大256ボーンに制限
	count = (count > 256) ? 256 : count;

	// 転置して送信
	std::vector<DirectX::XMMATRIX> transposed(count);
	for (int i = 0; i < count; i++)
	{
		transposed[i] = XMMatrixTranspose(matrices[i]);
	}

	m_DeviceContext->UpdateSubresource(m_BoneBuffer, 0, nullptr, transposed.data(), 0, 0);
	m_DeviceContext->VSSetConstantBuffers(4, 1, &m_BoneBuffer);
}

//==============================================================================
// ステート設定
//==============================================================================
void Renderer::SetDepthEnable(bool enable)
{
	if (enable)
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);
	else
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateDisable, 0);
}

void Renderer::SetDepthWriteEnable(bool enable)
{
	if (enable)
	{
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);
	}
	else
	{
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnableWriteDisable, 0);
	}
}

void Renderer::SetBlendMode(BlendMode mode)
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	switch (mode)
	{
	case BlendMode::None:
		m_DeviceContext->OMSetBlendState(m_BlendStateNone, blendFactor, 0xFFFFFFFF);
		break;
	case BlendMode::Alpha:
		m_DeviceContext->OMSetBlendState(m_BlendStateAlpha, blendFactor, 0xFFFFFFFF);
		break;
	case BlendMode::Add:
		m_DeviceContext->OMSetBlendState(m_BlendStateAdd, blendFactor, 0xFFFFFFFF);
		break;
	}
}

void Renderer::SetCullingMode(bool enableCulling)
{
	if (enableCulling)
		m_DeviceContext->RSSetState(m_RasterizerCullBack);
	else
		m_DeviceContext->RSSetState(m_RasterizerCullFront);
}

//==============================================================================
// 内部ヘルパー：深度ステンシルステート作成
//==============================================================================
bool Renderer::CreateDepthStencilStates()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = FALSE;

	// 深度テスト有効・書き込み有効（通常描画）
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	HRESULT hr = m_Device->CreateDepthStencilState(&dsDesc, &m_DepthStateEnable);
	if (FAILED(hr)) return false;

	// 深度テスト無効・書き込み無効（2D描画など）
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_Device->CreateDepthStencilState(&dsDesc, &m_DepthStateDisable);
	if (FAILED(hr)) return false;

	// 深度テスト有効・書き込み無効（パーティクル用）
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_Device->CreateDepthStencilState(&dsDesc, &m_DepthStateEnableWriteDisable);
	if (FAILED(hr)) return false;

	// デフォルトは深度有効
	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable, 0);

	return true;
}

//==============================================================================
// 内部ヘルパー：ブレンドステート作成
//==============================================================================
bool Renderer::CreateBlendStates()
{
	HRESULT hr;

	// ブレンド無効
	D3D11_BLEND_DESC blendNoneDesc = {};
	blendNoneDesc.RenderTarget[0].BlendEnable = FALSE;
	blendNoneDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = m_Device->CreateBlendState(&blendNoneDesc, &m_BlendStateNone);
	if (FAILED(hr)) return false;

	// 通常アルファブレンド
	D3D11_BLEND_DESC blendAlphaDesc = {};
	blendAlphaDesc.RenderTarget[0].BlendEnable = TRUE;
	blendAlphaDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendAlphaDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendAlphaDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendAlphaDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendAlphaDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendAlphaDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendAlphaDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = m_Device->CreateBlendState(&blendAlphaDesc, &m_BlendStateAlpha);
	if (FAILED(hr)) return false;

	// 加算合成
	D3D11_BLEND_DESC blendAddDesc = {};
	blendAddDesc.RenderTarget[0].BlendEnable = TRUE;
	blendAddDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendAddDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendAddDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendAddDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendAddDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendAddDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendAddDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = m_Device->CreateBlendState(&blendAddDesc, &m_BlendStateAdd);
	if (FAILED(hr)) return false;

	return true;
}

//==============================================================================
// 内部ヘルパー：ラスタライザステート作成
//==============================================================================
bool Renderer::CreateRasterizerStates()
{
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;

	// 背面カリング（通常）
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	HRESULT hr = m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerCullBack);
	if (FAILED(hr)) return false;

	// カリングなし
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerCullNone);
	if (FAILED(hr)) return false;

	// 前面カリング（アウトライン用）
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	hr = m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerCullFront);
	if (FAILED(hr)) return false;

	// デフォルトは背面カリング
	m_DeviceContext->RSSetState(m_RasterizerCullBack);

	return true;
}

//==============================================================================
// 内部ヘルパー：定数バッファ作成
//==============================================================================
bool Renderer::CreateConstantBuffers()
{
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	// World行列バッファ（4x4行列 = 64バイト）
	bufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX);
	hr = m_Device->CreateBuffer(&bufferDesc, nullptr, &m_WorldBuffer);
	if (FAILED(hr)) return false;

	// View行列バッファ
	hr = m_Device->CreateBuffer(&bufferDesc, nullptr, &m_ViewBuffer);
	if (FAILED(hr)) return false;

	// Projection行列バッファ
	hr = m_Device->CreateBuffer(&bufferDesc, nullptr, &m_ProjectionBuffer);
	if (FAILED(hr)) return false;

	// ボーン行列バッファ（b4）
	bufferDesc.ByteWidth = sizeof(DirectX::XMMATRIX) * 256;
	hr = m_Device->CreateBuffer(&bufferDesc, nullptr, &m_BoneBuffer);
	if (FAILED(hr)) return false;

	// マテリアルバッファ
	bufferDesc.ByteWidth = sizeof(MATERIAL);
	hr = m_Device->CreateBuffer(&bufferDesc, nullptr, &m_MaterialBuffer);
	if (FAILED(hr)) return false;

	// ライトバッファ
	bufferDesc.ByteWidth = sizeof(LIGHT);
	hr = m_Device->CreateBuffer(&bufferDesc, nullptr, &m_LightBuffer);
	if (FAILED(hr)) return false;

	// カメラ位置バッファ（float4 = 16バイト）
	bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);
	hr = m_Device->CreateBuffer(&bufferDesc, nullptr, &m_CameraBuffer);
	if (FAILED(hr)) return false;


	return true;
}

//==============================================================================
// サンプラーステート作成
//==============================================================================
bool Renderer::CreateSamplerState()
{
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = m_Device->CreateSamplerState(&samplerDesc, &m_SamplerState);
	if (FAILED(hr)) return false;

	// サンプラーをセット
	m_DeviceContext->PSSetSamplers(0, 1, &m_SamplerState);

	return true;
}

