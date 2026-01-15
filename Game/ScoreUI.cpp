//==============================================================================
// ScoreUI.cpp - スコア表示UI実装
//==============================================================================

#include "ScoreUI.h"
#include "ScoreManager.h"
#include "Core/Graphics/Renderer.h"

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
	m_NumberRenderer.Uninit();
}

//==============================================================================
// 初期化
//==============================================================================
bool ScoreUI::Init()
{
	return m_NumberRenderer.Init("Asset/Texture/number.png");
}

//==============================================================================
// 描画
//==============================================================================
void ScoreUI::Render()
{
	// 深度テスト無効（UI用）
	Renderer::SetDepthEnable(false);

	// 左チームスコア（左揃え）
	int leftScore = ScoreManager::Instance().GetLeftScore();
	m_NumberRenderer.DrawNumber(leftScore, leftScoreX, scoreY, digitWidth, digitHeight);

	// 右チームスコア（右揃え）
	int rightScore = ScoreManager::Instance().GetRightScore();
	m_NumberRenderer.DrawNumberRight(rightScore, rightScoreX, scoreY, digitWidth, digitHeight);

	// 深度テスト有効に戻す
	Renderer::SetDepthEnable(true);
}
