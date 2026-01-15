#pragma once

//==============================================================================
// NumberRenderer.h - 数字描画クラス
//==============================================================================

#include "Core/System/main.h"
#include "Core/Graphics/Texture.h"
#include <string>

//==============================================================================
// NumberRenderer クラス
//==============================================================================
class NumberRenderer
{
public:
	NumberRenderer();
	~NumberRenderer();

	// 初期化
	bool Init(const std::string& textureFile);
	void Uninit();

	// 数字描画
	void DrawNumber(int number, float x, float y, float digitWidth, float digitHeight);

	// 右揃えで描画
	void DrawNumberRight(int number, float rightX, float y, float digitWidth, float digitHeight);

	// 中央揃えで描画
	void DrawNumberCenter(int number, float centerX, float y, float digitWidth, float digitHeight);

private:
	ID3D11ShaderResourceView* m_Texture = nullptr;

	// テクスチャのグリッド設定
	static constexpr int GRID_COLS = 3;
	static constexpr int GRID_ROWS = 4;

	// 1桁描画
	void DrawDigit(int digit, float x, float y, float width, float height);

	// 桁数取得
	int GetDigitCount(int number);
};
