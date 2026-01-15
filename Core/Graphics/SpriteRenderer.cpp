//==============================================================================
// SpriteRenderer.cpp - スプライト描画コンポーネント実装
//==============================================================================

#include "SpriteRenderer.h"
#include "Texture.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"

//==============================================================================
// コンストラクタ
//==============================================================================
SpriteRenderer::SpriteRenderer()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
SpriteRenderer::~SpriteRenderer()
{
}

//==============================================================================
// 描画
//==============================================================================
void SpriteRenderer::Render()
{
	if (!m_Texture) return;

	Transform* transform = GetGameObject()->GetTransform();

	// 位置はTransformから、回転・スケールも反映
	float x = transform->position.x;
	float y = transform->position.y;
	float rotation = transform->GetEulerAngles().z * 0.0174533f;  // 度→ラジアン
	float scaleX = transform->scale.x;
	float scaleY = transform->scale.y;

	Sprite::DrawTransform(
		m_Texture,
		x, y,
		width, height,
		rotation,
		scaleX, scaleY,
		m_PivotX, m_PivotY,
		m_U, m_V, m_UW, m_VH,
		m_R, m_G, m_B, m_A
	);
}

//==============================================================================
// テクスチャ設定
//==============================================================================
void SpriteRenderer::SetTexture(ID3D11ShaderResourceView* texture)
{
	m_Texture = texture;
}

void SpriteRenderer::SetTexture(const std::string& fileName)
{
	m_Texture = Texture::Load(fileName);
}

//==============================================================================
// サイズ設定
//==============================================================================
void SpriteRenderer::SetSize(float w, float h)
{
	width = w;
	height = h;
}

//==============================================================================
// UV設定
//==============================================================================
void SpriteRenderer::SetUV(float u, float v, float uw, float vh)
{
	m_U = u;
	m_V = v;
	m_UW = uw;
	m_VH = vh;
}

//==============================================================================
// カラー設定
//==============================================================================
void SpriteRenderer::SetColor(float r, float g, float b, float a)
{
	m_R = r;
	m_G = g;
	m_B = b;
	m_A = a;
}

//==============================================================================
// ピボット設定
//==============================================================================
void SpriteRenderer::SetPivot(float x, float y)
{
	m_PivotX = x;
	m_PivotY = y;
}
