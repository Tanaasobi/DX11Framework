//==============================================================================
// ShadowMap.cpp - シャドウマップ実装
//==============================================================================

#include "ShadowMap.h"
#include "Renderer.h"
#include "Core/System/Logger.h"

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
ShadowMap::ShadowMap()
{
	m_LightView = XMMatrixIdentity();
	m_LightProjection = XMMatrixIdentity();
}

//==============================================================================
// デストラクタ
//==============================================================================
ShadowMap::~ShadowMap()
{
	Uninit();
}

//==============================================================================
// 初期化
//==============================================================================
bool ShadowMap::Init(int width, int height)
{
	m_Width = width;
	m_Height = height;

	ID3D11Device* device = Renderer::GetDevice();

	// シャドウマップテクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_ShadowMapTexture);
	if (FAILED(hr))
	{
		Logger::Error("Failed to create shadow map texture");
		return false;
	}

	// 深度ステンシルビュー作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(m_ShadowMapTexture, &dsvDesc, &m_ShadowMapDSV);
	if (FAILED(hr))
	{
		Logger::Error("Failed to create shadow map DSV");
		return false;
	}

	// シェーダーリソースビュー作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(m_ShadowMapTexture, &srvDesc, &m_ShadowMapSRV);
	if (FAILED(hr))
	{
		Logger::Error("Failed to create shadow map SRV");
		return false;
	}

	// ライト行列を更新
	UpdateLightMatrices();

	Logger::Info("ShadowMap initialized");
	return true;
}

//==============================================================================
// 終了
//==============================================================================
void ShadowMap::Uninit()
{
	SAFE_RELEASE(m_ShadowMapSRV);
	SAFE_RELEASE(m_ShadowMapDSV);
	SAFE_RELEASE(m_ShadowMapTexture);
}

//==============================================================================
// ライト方向設定
//==============================================================================
void ShadowMap::SetLightDirection(float x, float y, float z)
{
	m_LightDirection = XMFLOAT3(x, y, z);
	UpdateLightMatrices();
}

//==============================================================================
// ライトターゲット設定
//==============================================================================
void ShadowMap::SetLightTarget(float x, float y, float z)
{
	m_LightTarget = XMFLOAT3(x, y, z);
	UpdateLightMatrices();
}

//==============================================================================
// ライト行列更新
//==============================================================================
void ShadowMap::UpdateLightMatrices()
{
	// ライト位置（ターゲットから方向の逆に離れた位置）
	float distance = 50.0f;
	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&m_LightDirection));
	XMVECTOR target = XMLoadFloat3(&m_LightTarget);
	XMVECTOR lightPos = target - lightDir * distance;

	// ビュー行列
	m_LightView = XMMatrixLookAtLH(
		lightPos,
		target,
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	);

	// 正射影行列（シャドウマップ用）
	float orthoSize = 40.0f;  // フィールドサイズに合わせる
	m_LightProjection = XMMatrixOrthographicLH(
		orthoSize, orthoSize,
		1.0f, 100.0f
	);
}

//==============================================================================
// シャドウマップ描画開始
//==============================================================================
void ShadowMap::Begin()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// 現在のレンダーターゲットをバックアップ
	UINT numViewports = 1;
	context->RSGetViewports(&numViewports, &m_OldViewport);
	context->OMGetRenderTargets(1, &m_OldRTV, &m_OldDSV);

	// 現在のView/Projection行列をバックアップ
	// ※ Rendererから取得する方法が必要

	// シャドウマップをレンダーターゲットに設定
	ID3D11RenderTargetView* nullRTV = nullptr;
	context->OMSetRenderTargets(1, &nullRTV, m_ShadowMapDSV);

	// ビューポート設定
	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<float>(m_Width);
	vp.Height = static_cast<float>(m_Height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	// シャドウマップをクリア
	context->ClearDepthStencilView(m_ShadowMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// ライト視点の行列を設定
	Renderer::SetViewMatrix(m_LightView);
	Renderer::SetProjectionMatrix(m_LightProjection);
}

//==============================================================================
// シャドウマップ描画終了
//==============================================================================
void ShadowMap::End()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// 元のレンダーターゲットに戻す
	context->OMSetRenderTargets(1, &m_OldRTV, m_OldDSV);
	context->RSSetViewports(1, &m_OldViewport);

	SAFE_RELEASE(m_OldRTV);
	SAFE_RELEASE(m_OldDSV);

	// ※ View/Projectionは呼び出し側で復元する必要がある
}
