//==============================================================================
// CountdownUI.cpp - カウントダウン表示実装
//==============================================================================

#include "CountdownUI.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Graphics/Sprite.h"
#include "Core/Graphics/Texture.h"
#include "Core/System/main.h"

//==============================================================================
// コンストラクタ
//==============================================================================
CountdownUI::CountdownUI()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
CountdownUI::~CountdownUI()
{
	m_NumberRenderer.Uninit();
}

//==============================================================================
// 初期化
//==============================================================================
bool CountdownUI::Init()
{
	return m_NumberRenderer.Init("Asset/Texture/number.png");
}

//==============================================================================
// 描画
//==============================================================================
void CountdownUI::Render()
{
	if (m_Count < 0 && !m_ShowGo) return;

	Renderer::SetDepthEnable(false);

	float centerX = SCREEN_WIDTH * 0.5f;
	float centerY = SCREEN_HEIGHT * 0.5f;
	float digitWidth = 120.0f;
	float digitHeight = 160.0f;

	if (m_ShowGo)
	{
		// "GO!" は数字の0を大きく表示（本来はテクスチャで"GO!"を用意）
		// ここでは簡易的に何も表示しないか、別のテクスチャを使う
		// 数字テクスチャを流用して0を表示
		m_NumberRenderer.DrawNumberCenter(0, centerX, centerY - digitHeight * 0.5f, digitWidth, digitHeight);
	}
	else if (m_Count > 0)
	{
		// カウントダウン数字
		m_NumberRenderer.DrawNumberCenter(m_Count, centerX, centerY - digitHeight * 0.5f, digitWidth, digitHeight);
	}

	Renderer::SetDepthEnable(true);
}

//==============================================================================
// カウント設定
//==============================================================================
void CountdownUI::SetCount(int count)
{
	m_Count = count;
	m_ShowGo = (count == 0);
}

//==============================================================================
// 非表示
//==============================================================================
void CountdownUI::Hide()
{
	m_Count = -1;
	m_ShowGo = false;
}
