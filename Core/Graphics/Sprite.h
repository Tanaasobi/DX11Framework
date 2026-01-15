#pragma once

//==============================================================================
// Sprite.h - スプライト描画クラス
//==============================================================================

#include "Core/System/main.h"
#include "Texture.h"

//==============================================================================
// スプライト用定数バッファ構造体
//==============================================================================
struct SPRITE_BUFFER
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT4 UVOffset;  // x, y = オフセット, z, w = スケール
};

//==============================================================================
// Sprite クラス
// - 2D スプライト描画
// - 静的クラスとして実装
//==============================================================================
class Sprite
{
public:
	// 初期化・終了
	static bool Init();
	static void Uninit();

	// 描画（ピクセル座標指定）
	static void Draw(
		ID3D11ShaderResourceView* texture,
		float x, float y,
		float width, float height,
		float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f
	);

	// 描画（UV指定あり、アニメーション用）
	static void DrawUV(
		ID3D11ShaderResourceView* texture,
		float x, float y,
		float width, float height,
		float u, float v,
		float uw, float vh,
		float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f
	);


	// 描画（回転・スケール対応）
	static void DrawTransform(
		ID3D11ShaderResourceView* texture,
		float x, float y,
		float width, float height,
		float rotation,           // ラジアン
		float scaleX, float scaleY,
		float pivotX, float pivotY, // ピボット（0.0〜1.0、0.5で中心）
		float u, float v,
		float uw, float vh,
		float r, float g, float b, float a
	);

private:
	static ID3D11VertexShader* m_VertexShader;
	static ID3D11PixelShader* m_PixelShader;
	static ID3D11InputLayout* m_InputLayout;
	static ID3D11Buffer* m_VertexBuffer;
	static ID3D11Buffer* m_ConstantBuffer;

	static bool CreateShaders();
	static bool CreateBuffers();
};
