#include "ResultScene.h"
#include "Game/System/ScoreManager.h"
#include "Core/System/Input.h"
#include "Core/Scene/SceneManager.h"
#include "Core/Graphics/UI/TextRenderer.h"
#include "Core/System/main.h"
#include <string>

void ResultScene::Init()
{
	m_Font = new Font();
	m_Font->Init(L"Yu Gothic", 64.0f);
}

void ResultScene::Update(float deltaTime)
{
	// 入力でタイトルへ
	if (Input::GetKeyDown(KeyCode::Space) || Input::GetKeyDown(KeyCode::MouseLeft) ||
		Input::GetGamepadButtonDown(0, GamepadButton::A))
	{
		SceneManager::LoadScene("TitleScene");
	}
}

void ResultScene::Render()
{
	if (!m_Font) return;

	int leftScore = ScoreManager::Instance().GetLeftScore();
	int rightScore = ScoreManager::Instance().GetRightScore();

	std::wstring winnerText;
	if (leftScore > rightScore) winnerText = L"WINNER: PLAYER 1"; // 左
	else if (rightScore > leftScore) winnerText = L"WINNER: CPU";  // 右
	else winnerText = L"DRAW";

	// 結果表示
	TextRenderer::Draw(m_Font, winnerText,
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 100,
		1.2f, 1.0f, 1.0f, 0.0f, TextAlign::Center); // 黄色

	// スコア表示
	std::wstring scoreText = std::to_wstring(leftScore) + L" - " + std::to_wstring(rightScore);
	TextRenderer::Draw(m_Font, scoreText,
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
		2.0f, 1.0f, 1.0f, 1.0f, TextAlign::Center);

	// メッセージ
	TextRenderer::Draw(m_Font, L"PRESS SPACE TO TITLE",
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 150,
		0.6f, 0.8f, 0.8f, 0.8f, TextAlign::Center);
}
