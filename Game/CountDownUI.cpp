//==============================================================================
// CountdownUI.cpp - カウントダウン表示実装
//==============================================================================

#include "CountdownUI.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Graphics/UI/TextRenderer.h"
#include "Core/System/main.h"
#include <string>

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
	if (m_Font)
	{
		m_Font->Uninit();
		delete m_Font;
		m_Font = nullptr;
	}
}

//==============================================================================
// 初期化
//==============================================================================
bool CountdownUI::Init()
{
	m_Font = new Font();
	return m_Font->Init(L"Yu Gothic", 128.0f);
}

//==============================================================================
// 描画
//==============================================================================
void CountdownUI::Render()
{
	if (m_Count < 0 && !m_ShowGo) return;
	if (!m_Font) return;

	float centerX = SCREEN_WIDTH * 0.5f;
	float centerY = SCREEN_HEIGHT * 0.5f;

	if (m_ShowGo)
	{
		// "GO!" を表示
		TextRenderer::Draw(m_Font, L"GO!", centerX, centerY - m_Font->GetLineHeight() * 0.5f,
			1.0f, 1.0f, 0.0f, 1.0f, TextAlign::Center);
	}
	else if (m_Count > 0)
	{
		// カウントダウン数字
		std::wstring countText = std::to_wstring(m_Count);
		TextRenderer::Draw(m_Font, countText, centerX, centerY - m_Font->GetLineHeight() * 0.5f,
			1.0f, 1.0f, 1.0f, 1.0f, TextAlign::Center);
	}
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
