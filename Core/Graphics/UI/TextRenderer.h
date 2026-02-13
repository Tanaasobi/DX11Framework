#pragma once

//==============================================================================
// TextRenderer.h - テキスト描画管理クラス
//==============================================================================

#include "Core/System/main.h"
#include <dwrite.h>
#include <d2d1.h>
#include <wincodec.h>

class Font;

//==============================================================================
// テキスト配置
//==============================================================================
enum class TextAlign
{
	Left,    // 左揃え
	Center,  // 中央揃え
	Right,   // 右揃え
};

//==============================================================================
// TextRenderer クラス
// - DirectWrite/Direct2D ファクトリの管理
// - テキスト描画の静的APIを提供
// - 全て静的メンバとして実装
//==============================================================================
class TextRenderer
{
public:
	//--------------------------------------------------------------------------
	// 初期化・終了
	//--------------------------------------------------------------------------
	static bool Init();
	static void Uninit();

	//--------------------------------------------------------------------------
	// 描画（wstring版、日本語対応）
	//--------------------------------------------------------------------------
	static void Draw(
		Font* font,
		const std::wstring& text,
		float x, float y,
		float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f,
		TextAlign align = TextAlign::Left
	);

	//--------------------------------------------------------------------------
	// 描画（string版、内部でwstringに変換）
	//--------------------------------------------------------------------------
	static void Draw(
		Font* font,
		const std::string& text,
		float x, float y,
		float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f,
		TextAlign align = TextAlign::Left
	);

	//--------------------------------------------------------------------------
	// テキスト幅の計測
	//--------------------------------------------------------------------------
	static float MeasureWidth(Font* font, const std::wstring& text);

	//--------------------------------------------------------------------------
	// ファクトリ取得（Fontクラスから使用）
	//--------------------------------------------------------------------------
	static IDWriteFactory* GetDWriteFactory() { return m_DWriteFactory; }
	static ID2D1Factory* GetD2DFactory() { return m_D2DFactory; }
	static IWICImagingFactory* GetWICFactory() { return m_WICFactory; }

private:
	static IDWriteFactory* m_DWriteFactory;
	static ID2D1Factory* m_D2DFactory;
	static IWICImagingFactory* m_WICFactory;

	// マルチバイトをワイド文字列に変換
	static std::wstring MultiByteToWide(const std::string& str);
};
