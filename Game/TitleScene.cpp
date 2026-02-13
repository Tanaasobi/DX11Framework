#include "TitleScene.h"
#include "Core/System/Input.h"
#include "Core/Scene/SceneManager.h"
#include "Core/System/Time.h"
#include "Core/Graphics/UI/TextRenderer.h"
#include "Core/System/main.h" // SCREEN_WIDTH, SCREEN_HEIGHT

void TitleScene::Init()
{
	m_Font = new Font();
	m_Font->Init(L"Yu Gothic", 64.0f);
}

void TitleScene::Update(float deltaTime)
{
	// スペースキーかクリックでゲーム開始
	if (Input::GetKeyDown(KeyCode::Space) || Input::GetKeyDown(KeyCode::MouseLeft) ||
		Input::GetGamepadButtonDown(0, GamepadButton::A))
	{
		SceneManager::LoadScene("GameScene");
	}
}

void TitleScene::Render()
{
	if (!m_Font) return;

	// タイトル表示
	TextRenderer::Draw(m_Font, L"AIR HOCKEY",
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 150,
		1.0f, 1.0f, 1.0f, 1.0f,
		TextAlign::Center);

	// 点滅演出
	float alpha = (sinf(Time::GetTotalTime() * 5.0f) + 1.0f) * 0.5f;
	TextRenderer::Draw(m_Font, L"PRESS SPACE TO START",
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 100,
		1.0f, 1.0f, 1.0f, alpha, 
		TextAlign::Center);
}
