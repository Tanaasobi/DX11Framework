//==============================================================================
// NumberRenderer.cpp - 数字描画クラス実装
//==============================================================================

#include "NumberRenderer.h"
#include "Core/Graphics/Sprite.h"
#include <cmath>

//==============================================================================
// コンストラクタ
//==============================================================================
NumberRenderer::NumberRenderer()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
NumberRenderer::~NumberRenderer()
{
	Uninit();
}

//==============================================================================
// 初期化
//==============================================================================
bool NumberRenderer::Init(const std::string& textureFile)
{
	m_Texture = Texture::Load(textureFile);
	return (m_Texture != nullptr);
}

//==============================================================================
// 終了
//==============================================================================
void NumberRenderer::Uninit()
{
	// Textureはキャッシュで管理されるので解放しない
	m_Texture = nullptr;
}

//==============================================================================
// 1桁描画
//==============================================================================
void NumberRenderer::DrawDigit(int digit, float x, float y, float width, float height)
{
	if (!m_Texture || digit < 0 || digit > 9) return;

	// UV計算
	int col = digit % GRID_COLS;
	int row = digit / GRID_COLS;

	float uWidth = 1.0f / GRID_COLS;
	float vHeight = 1.0f / GRID_ROWS;

	float u = col * uWidth;
	float v = row * vHeight;

	// 既存のSprite::Drawを使用（u, v, uw, vh形式）
	Sprite::DrawUV(m_Texture, x, y, width, height, u, v, uWidth, vHeight);
}

//==============================================================================
// 数字描画（左揃え）
//==============================================================================
void NumberRenderer::DrawNumber(int number, float x, float y, float digitWidth, float digitHeight)
{
	if (number < 0) number = 0;

	// 0の場合
	if (number == 0)
	{
		DrawDigit(0, x, y, digitWidth, digitHeight);
		return;
	}

	// 桁ごとに描画
	int temp = number;
	int digitCount = GetDigitCount(number);

	// 最上位桁から描画
	int divisor = 1;
	for (int i = 1; i < digitCount; i++)
	{
		divisor *= 10;
	}

	float currentX = x;
	while (divisor > 0)
	{
		int digit = temp / divisor;
		DrawDigit(digit, currentX, y, digitWidth, digitHeight);
		temp %= divisor;
		divisor /= 10;
		currentX += digitWidth;
	}
}

//==============================================================================
// 数字描画（右揃え）
//==============================================================================
void NumberRenderer::DrawNumberRight(int number, float rightX, float y, float digitWidth, float digitHeight)
{
	int digitCount = GetDigitCount(number);
	float startX = rightX - digitCount * digitWidth;
	DrawNumber(number, startX, y, digitWidth, digitHeight);
}

//==============================================================================
// 数字描画（中央揃え）
//==============================================================================
void NumberRenderer::DrawNumberCenter(int number, float centerX, float y, float digitWidth, float digitHeight)
{
	int digitCount = GetDigitCount(number);
	float startX = centerX - (digitCount * digitWidth) * 0.5f;
	DrawNumber(number, startX, y, digitWidth, digitHeight);
}

//==============================================================================
// 桁数取得
//==============================================================================
int NumberRenderer::GetDigitCount(int number)
{
	if (number == 0) return 1;
	if (number < 0) number = -number;

	int count = 0;
	while (number > 0)
	{
		count++;
		number /= 10;
	}
	return count;
}
