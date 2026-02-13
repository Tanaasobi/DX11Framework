//==============================================================================
// TextRenderer.cpp - テキスト描画管理クラス実装
//==============================================================================

#include "TextRenderer.h"
#include "Font.h"
#include "Core/Graphics/Sprite.h"
#include "Core/System/Logger.h"

//==============================================================================
// 静的メンバ定義
//==============================================================================
IDWriteFactory* TextRenderer::m_DWriteFactory = nullptr;
ID2D1Factory* TextRenderer::m_D2DFactory = nullptr;
IWICImagingFactory* TextRenderer::m_WICFactory = nullptr;

//==============================================================================
// 初期化
//==============================================================================
bool TextRenderer::Init()
{
	HRESULT hr;

	// DirectWrite ファクトリ作成
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_DWriteFactory)
	);
	if (FAILED(hr))
	{
		Logger::Error("TextRenderer: DWriteCreateFactory failed");
		return false;
	}

	// Direct2D ファクトリ作成
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_D2DFactory);
	if (FAILED(hr))
	{
		Logger::Error("TextRenderer: D2D1CreateFactory failed");
		SAFE_RELEASE(m_DWriteFactory);
		return false;
	}

	// WIC ファクトリ作成
	hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_WICFactory)
	);
	if (FAILED(hr))
	{
		Logger::Error("TextRenderer: WIC factory creation failed");
		SAFE_RELEASE(m_D2DFactory);
		SAFE_RELEASE(m_DWriteFactory);
		return false;
	}

	Logger::Info("TextRenderer initialized");
	return true;
}

//==============================================================================
// 終了
//==============================================================================
void TextRenderer::Uninit()
{
	SAFE_RELEASE(m_WICFactory);
	SAFE_RELEASE(m_D2DFactory);
	SAFE_RELEASE(m_DWriteFactory);

	Logger::Info("TextRenderer uninitialized");
}

//==============================================================================
// 描画（wstring版）
//==============================================================================
void TextRenderer::Draw(
	Font* font,
	const std::wstring& text,
	float x, float y,
	float r, float g, float b, float a,
	TextAlign align)
{
	if (!font || text.empty()) return;

	// テキストをレンダリング（キャッシュから取得 or 新規作成）
	TextCacheEntry* entry = font->GetOrRenderText(text);
	if (!entry || !entry->srv) return;

	// アライメントに応じてX座標を調整
	float drawX = x;
	switch (align)
	{
	case TextAlign::Left:
		break;
	case TextAlign::Center:
		drawX = x - entry->width * 0.5f;
		break;
	case TextAlign::Right:
		drawX = x - entry->width;
		break;
	}

	// 既存のSpriteシステムで描画
	Sprite::Draw(entry->srv, drawX, y, entry->width, entry->height, r, g, b, a);
}

//==============================================================================
// 描画（string版）
//==============================================================================
void TextRenderer::Draw(
	Font* font,
	const std::string& text,
	float x, float y,
	float r, float g, float b, float a,
	TextAlign align)
{
	Draw(font, MultiByteToWide(text), x, y, r, g, b, a, align);
}

//==============================================================================
// テキスト幅の計測
//==============================================================================
float TextRenderer::MeasureWidth(Font* font, const std::wstring& text)
{
	if (!font || text.empty()) return 0.0f;
	return font->MeasureWidth(text);
}

//==============================================================================
// マルチバイトをワイド文字列に変換
//==============================================================================
std::wstring TextRenderer::MultiByteToWide(const std::string& str)
{
	if (str.empty()) return std::wstring();

	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	std::wstring wstr(size - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
	return wstr;
}
