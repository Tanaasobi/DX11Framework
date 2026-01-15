#pragma once

//==============================================================================
// ScoreManager.h - スコア管理
//==============================================================================

#include "Goal.h"

//==============================================================================
// ScoreManager クラス（シングルトン）
//==============================================================================
class ScoreManager
{
public:
	static ScoreManager& Instance()
	{
		static ScoreManager instance;
		return instance;
	}

	// スコア操作
	void Reset();
	void AddScore(Team team, int points = 1);

	// スコア取得
	int GetScore(Team team) const;
	int GetLeftScore() const { return m_LeftScore; }
	int GetRightScore() const { return m_RightScore; }

	// 勝利判定
	bool IsGameOver() const;
	Team GetWinner() const;

	// 設定
	int winScore = 5;  // 勝利に必要なスコア

private:
	ScoreManager() { Reset(); }
	~ScoreManager() = default;

	ScoreManager(const ScoreManager&) = delete;
	ScoreManager& operator=(const ScoreManager&) = delete;

	int m_LeftScore = 0;
	int m_RightScore = 0;
};
