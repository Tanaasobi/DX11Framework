#pragma once

//==============================================================================
// SpriteRenderer.h - スプライト描画コンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "Sprite.h"

//==============================================================================
// SpriteRenderer クラス
//==============================================================================
class SpriteRenderer : public Component
{
public:
	SpriteRenderer();
	virtual ~SpriteRenderer();

	void Render() override;

	// テクスチャ設定
	void SetTexture(ID3D11ShaderResourceView* texture);
	void SetTexture(const std::string& fileName);

	// サイズ設定
	void SetSize(float width, float height);

	// UV設定（アニメーション用）
	void SetUV(float u, float v, float uw, float vh);

	// カラー設定
	void SetColor(float r, float g, float b, float a = 1.0f);

	// ピボット設定（0.0〜1.0、0.5で中心）
	void SetPivot(float x, float y);

public:
	float width = 100.0f;
	float height = 100.0f;

private:
	ID3D11ShaderResourceView* m_Texture = nullptr;

	float m_U = 0.0f;
	float m_V = 0.0f;
	float m_UW = 1.0f;
	float m_VH = 1.0f;

	float m_R = 1.0f;
	float m_G = 1.0f;
	float m_B = 1.0f;
	float m_A = 1.0f;

	float m_PivotX = 0.0f;
	float m_PivotY = 0.0f;
};
