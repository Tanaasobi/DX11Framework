#pragma once

//==============================================================================
// CountdownUI.h - カウントダウン表示
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/Graphics/UI/Font.h"

class CountdownUI : public Component
{
public:
	CountdownUI();
	virtual ~CountdownUI();

	bool Init();
	void Render() override;

	// 表示する数字（0以下で非表示、0="GO!"）
	void SetCount(int count);
	void Hide();

private:
	Font* m_Font = nullptr;
	int m_Count = -1;
	bool m_ShowGo = false;
};
