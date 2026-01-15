#pragma once

//==============================================================================
// ShadowMap.h - シャドウマップ
//==============================================================================

#include "Core/System/main.h"
#include <DirectXMath.h>

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

	bool Init(int width, int height);
	void Uninit();

	// シャドウマップ描画開始・終了
	void Begin();
	void End();

	// ライト設定
	void SetLightDirection(float x, float y, float z);
	void SetLightTarget(float x, float y, float z);

	// ゲッター
	ID3D11ShaderResourceView* GetShadowMapSRV() const { return m_ShadowMapSRV; }
	const DirectX::XMMATRIX& GetLightViewMatrix() const { return m_LightView; }
	const DirectX::XMMATRIX& GetLightProjectionMatrix() const { return m_LightProjection; }

	// シャドウマップサイズ
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

private:
	int m_Width = 2048;
	int m_Height = 2048;

	ID3D11Texture2D* m_ShadowMapTexture = nullptr;
	ID3D11DepthStencilView* m_ShadowMapDSV = nullptr;
	ID3D11ShaderResourceView* m_ShadowMapSRV = nullptr;

	// ライト用行列
	DirectX::XMMATRIX m_LightView;
	DirectX::XMMATRIX m_LightProjection;

	// ライト位置・方向
	DirectX::XMFLOAT3 m_LightDirection = { -1.0f, -1.0f, 1.0f };
	DirectX::XMFLOAT3 m_LightTarget = { 0.0f, 0.0f, 0.0f };

	// バックアップ用
	ID3D11RenderTargetView* m_OldRTV = nullptr;
	ID3D11DepthStencilView* m_OldDSV = nullptr;
	D3D11_VIEWPORT m_OldViewport = {};
	DirectX::XMMATRIX m_OldView;
	DirectX::XMMATRIX m_OldProjection;

	void UpdateLightMatrices();
};
