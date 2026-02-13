#pragma once

//==============================================================================
// ScoreUI.h - スコア表示UI
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/Graphics/UI/Font.h"

//==============================================================================
// ScoreUI クラス
//==============================================================================
class ScoreUI : public Component
{
public:
	ScoreUI();
	virtual ~ScoreUI();

	bool Init();
	void Render() override;

	// 表示位置
	float leftScoreX = 100.0f;
	float rightScoreX = 1820.0f;
	float scoreY = 50.0f;

private:
	Font* m_Font = nullptr;
};
