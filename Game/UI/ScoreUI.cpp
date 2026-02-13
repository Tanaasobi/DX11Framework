//==============================================================================
// ScoreUI.cpp - スコア表示UI実装
//==============================================================================

#include "ScoreUI.h"
#include "Game/System/ScoreManager.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Graphics/UI/TextRenderer.h"
#include <string>

//==============================================================================
// コンストラクタ
//==============================================================================
ScoreUI::ScoreUI()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
ScoreUI::~ScoreUI()
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
bool ScoreUI::Init()
{
	m_Font = new Font();
	return m_Font->Init(L"Yu Gothic", 64.0f);
}

//==============================================================================
// 描画
//==============================================================================
void ScoreUI::Render()
{
	if (!m_Font) return;

	// 左チームスコア（左揃え）
	int leftScore = ScoreManager::Instance().GetLeftScore();
	std::wstring leftText = std::to_wstring(leftScore);
	TextRenderer::Draw(m_Font, leftText, leftScoreX, scoreY, 1.0f, 1.0f, 1.0f, 1.0f, TextAlign::Left);

	// 右チームスコア（右揃え）
	int rightScore = ScoreManager::Instance().GetRightScore();
	std::wstring rightText = std::to_wstring(rightScore);
	TextRenderer::Draw(m_Font, rightText, rightScoreX, scoreY, 1.0f, 1.0f, 1.0f, 1.0f, TextAlign::Right);
}
