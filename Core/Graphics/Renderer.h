#pragma once

//==============================================================================
// Renderer.h - DirectX11 レンダリング管理クラス
//==============================================================================

#include "../System/main.h"

//==============================================================================
// ブレンドモード
//==============================================================================
enum class BlendMode
{
    None,       // ブレンド無効
    Alpha,      // 通常アルファブレンド
    Add,        // 加算合成
};

//==============================================================================
// Renderer クラス
// - DirectX11 デバイス・コンテキスト・スワップチェインの管理
// - レンダーターゲット・深度ステンシルの管理
// - 各種ステート・定数バッファの管理
// - 全て静的メンバとして実装
//==============================================================================
class Renderer
{
public:
    //--------------------------------------------------------------------------
    // 初期化・終了
    //--------------------------------------------------------------------------
    static bool Init(HWND hWnd);
    static void Uninit();

    //--------------------------------------------------------------------------
    // 描画フロー
    //--------------------------------------------------------------------------
    static void Begin();
    static void End();

    //--------------------------------------------------------------------------
    // デバイス・コンテキスト取得
    //--------------------------------------------------------------------------
    static ID3D11Device* GetDevice() { return m_Device; }
    static ID3D11DeviceContext* GetDeviceContext() { return m_DeviceContext; }

    //--------------------------------------------------------------------------
    // 定数バッファ更新
    //--------------------------------------------------------------------------
    static void SetWorldMatrix(const DirectX::XMMATRIX& worldMatrix);
    static void SetViewMatrix(const DirectX::XMMATRIX& viewMatrix);
    static void SetProjectionMatrix(const DirectX::XMMATRIX& projectionMatrix);
	static void SetBoneMatrices(const DirectX::XMMATRIX* matrices, int count);
    static void SetMaterial(const MATERIAL& material);
    static void SetLight(const LIGHT& light);
    static void SetCameraPosition(const DirectX::XMFLOAT3& cameraPosition);

    //--------------------------------------------------------------------------
    // ステート設定
    //--------------------------------------------------------------------------
    static void SetDepthEnable(bool enable);
	static void SetDepthWriteEnable(bool enable);
    static void SetBlendMode(BlendMode mode);
    static void SetCullingMode(bool enableCulling);

private:
    //--------------------------------------------------------------------------
    // DirectX11 オブジェクト
    //--------------------------------------------------------------------------

    // デバイス関連
    static ID3D11Device* m_Device;
    static ID3D11DeviceContext* m_DeviceContext;
    static IDXGISwapChain* m_SwapChain;

    // レンダーターゲット
    static ID3D11RenderTargetView* m_RenderTargetView;
    static ID3D11DepthStencilView* m_DepthStencilView;

    // 定数バッファ
    static ID3D11Buffer* m_WorldBuffer;
    static ID3D11Buffer* m_ViewBuffer;
    static ID3D11Buffer* m_ProjectionBuffer;
	static ID3D11Buffer* m_BoneBuffer;
    static ID3D11Buffer* m_MaterialBuffer;
    static ID3D11Buffer* m_LightBuffer;
    static ID3D11Buffer* m_CameraBuffer;

    // 深度ステンシルステート
    static ID3D11DepthStencilState* m_DepthStateEnable;
    static ID3D11DepthStencilState* m_DepthStateDisable;
	static ID3D11DepthStencilState* m_DepthStateEnableWriteDisable;

    // ブレンドステート
    static ID3D11BlendState* m_BlendStateNone;
    static ID3D11BlendState* m_BlendStateAlpha;
    static ID3D11BlendState* m_BlendStateAdd;

	// ラスタライザステート
	static ID3D11RasterizerState* m_RasterizerCullBack;
	static ID3D11RasterizerState* m_RasterizerCullFront;
	static ID3D11RasterizerState* m_RasterizerCullNone;

	// サンプラーステート
	static ID3D11SamplerState* m_SamplerState;

    //--------------------------------------------------------------------------
    // 内部ヘルパー
    //--------------------------------------------------------------------------
    static bool CreateDepthStencilStates();
    static bool CreateBlendStates();
    static bool CreateRasterizerStates();
    static bool CreateConstantBuffers();
	static bool CreateSamplerState();
};
