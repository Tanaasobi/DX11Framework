//==============================================================================
// ScoreManager.cpp - スコア管理実装
//==============================================================================

#include "ScoreManager.h"
#include "Core/System/Logger.h"

//==============================================================================
// リセット
//==============================================================================
void ScoreManager::Reset()
{
	m_LeftScore = 0;
	m_RightScore = 0;
}

//==============================================================================
// スコア加算
//==============================================================================
void ScoreManager::AddScore(Team team, int points)
{
	if (team == Team::Left)
	{
		m_LeftScore += points;
		Logger::InfoFormat("Left Team scored! Score: %d - %d", m_LeftScore, m_RightScore);
	}
	else
	{
		m_RightScore += points;
		Logger::InfoFormat("Right Team scored! Score: %d - %d", m_LeftScore, m_RightScore);
	}
}

//==============================================================================
// スコア取得
//==============================================================================
int ScoreManager::GetScore(Team team) const
{
	return (team == Team::Left) ? m_LeftScore : m_RightScore;
}

//==============================================================================
// ゲーム終了判定
//==============================================================================
bool ScoreManager::IsGameOver() const
{
	return m_LeftScore >= winScore || m_RightScore >= winScore;
}

//==============================================================================
// 勝者取得
//==============================================================================
Team ScoreManager::GetWinner() const
{
	return (m_LeftScore >= winScore) ? Team::Left : Team::Right;
}
