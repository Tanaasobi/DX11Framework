//==============================================================================
// Font.cpp - フォントクラス実装
//==============================================================================

#include "Font.h"
#include "TextRenderer.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include <cmath>

//==============================================================================
// コンストラクタ
//==============================================================================
Font::Font()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Font::~Font()
{
	Uninit();
}

//==============================================================================
// 初期化
//==============================================================================
bool Font::Init(const std::wstring& fontName, float fontSize)
{
	IDWriteFactory* dwFactory = TextRenderer::GetDWriteFactory();
	if (!dwFactory)
	{
		Logger::Error("Font::Init: DWriteFactory is null");
		return false;
	}

	m_FontName = fontName;
	m_FontSize = fontSize;

	// テキストフォーマット作成
	HRESULT hr = dwFactory->CreateTextFormat(
		fontName.c_str(),
		nullptr,                         // フォントコレクション（システムデフォルト）
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"ja-jp",                        // ロケール
		&m_TextFormat
	);

	if (FAILED(hr))
	{
		Logger::Error("Font::Init: CreateTextFormat failed");
		return false;
	}

	// 行の高さを取得
	IDWriteTextLayout* tempLayout = nullptr;
	hr = dwFactory->CreateTextLayout(
		L"A", 1, m_TextFormat,
		1000.0f, 1000.0f,
		&tempLayout
	);

	if (SUCCEEDED(hr) && tempLayout)
	{
		DWRITE_TEXT_METRICS metrics;
		tempLayout->GetMetrics(&metrics);
		m_LineHeight = metrics.height;
		tempLayout->Release();
	}
	else
	{
		m_LineHeight = fontSize * 1.2f;
	}

	Logger::Info("Font initialized");
	return true;
}

//==============================================================================
// 終了
//==============================================================================
void Font::Uninit()
{
	ClearCache();
	SAFE_RELEASE(m_TextFormat);
}

//==============================================================================
// テキストのレンダリングとキャッシュ取得
//==============================================================================
TextCacheEntry* Font::GetOrRenderText(const std::wstring& text)
{
	if (!m_TextFormat || text.empty()) return nullptr;

	// キャッシュから検索
	auto it = m_TextCache.find(text);
	if (it != m_TextCache.end())
	{
		return &it->second;
	}

	// キャッシュ上限に達した場合はクリア
	if (static_cast<int>(m_TextCache.size()) >= MAX_CACHE_ENTRIES)
	{
		ClearCache();
	}

	// 新規レンダリング
	TextCacheEntry entry = RenderTextToTexture(text);
	if (!entry.srv) return nullptr;

	// キャッシュに格納
	m_TextCache[text] = entry;
	return &m_TextCache[text];
}

//==============================================================================
// テキスト幅の計測
//==============================================================================
float Font::MeasureWidth(const std::wstring& text)
{
	if (!m_TextFormat || text.empty()) return 0.0f;

	IDWriteFactory* dwFactory = TextRenderer::GetDWriteFactory();
	if (!dwFactory) return 0.0f;

	IDWriteTextLayout* textLayout = nullptr;
	HRESULT hr = dwFactory->CreateTextLayout(
		text.c_str(),
		static_cast<UINT32>(text.length()),
		m_TextFormat,
		4096.0f, 4096.0f,
		&textLayout
	);

	if (FAILED(hr) || !textLayout) return 0.0f;

	DWRITE_TEXT_METRICS metrics;
	textLayout->GetMetrics(&metrics);
	float width = metrics.widthIncludingTrailingWhitespace;

	textLayout->Release();
	return width;
}

//==============================================================================
// キャッシュクリア
//==============================================================================
void Font::ClearCache()
{
	for (auto& pair : m_TextCache)
	{
		SAFE_RELEASE(pair.second.srv);
	}
	m_TextCache.clear();
}

//==============================================================================
// テキストをテクスチャにレンダリング
//==============================================================================
TextCacheEntry Font::RenderTextToTexture(const std::wstring& text)
{
	TextCacheEntry entry = {};

	IDWriteFactory* dwFactory = TextRenderer::GetDWriteFactory();
	ID2D1Factory* d2dFactory = TextRenderer::GetD2DFactory();
	IWICImagingFactory* wicFactory = TextRenderer::GetWICFactory();

	if (!dwFactory || !d2dFactory || !wicFactory) return entry;

	// 1. テキストレイアウト作成
	IDWriteTextLayout* textLayout = nullptr;
	HRESULT hr = dwFactory->CreateTextLayout(
		text.c_str(),
		static_cast<UINT32>(text.length()),
		m_TextFormat,
		4096.0f,  // 最大幅
		4096.0f,  // 最大高さ
		&textLayout
	);
	if (FAILED(hr) || !textLayout)
	{
		Logger::Error("Font: CreateTextLayout failed");
		return entry;
	}

	// 2. テキストメトリクスを取得
	DWRITE_TEXT_METRICS metrics;
	textLayout->GetMetrics(&metrics);

	UINT32 bitmapWidth = static_cast<UINT32>(ceilf(metrics.widthIncludingTrailingWhitespace)) + 2;
	UINT32 bitmapHeight = static_cast<UINT32>(ceilf(metrics.height)) + 2;

	if (bitmapWidth == 0 || bitmapHeight == 0)
	{
		textLayout->Release();
		return entry;
	}

	entry.width = static_cast<float>(bitmapWidth);
	entry.height = static_cast<float>(bitmapHeight);

	// 3. WICビットマップ作成
	IWICBitmap* wicBitmap = nullptr;
	hr = wicFactory->CreateBitmap(
		bitmapWidth, bitmapHeight,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapCacheOnDemand,
		&wicBitmap
	);
	if (FAILED(hr))
	{
		Logger::Error("Font: WIC CreateBitmap failed");
		textLayout->Release();
		return entry;
	}

	// 4. D2Dレンダーターゲット作成
	D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	ID2D1RenderTarget* rt = nullptr;
	hr = d2dFactory->CreateWicBitmapRenderTarget(wicBitmap, rtProps, &rt);
	if (FAILED(hr))
	{
		Logger::Error("Font: CreateWicBitmapRenderTarget failed");
		wicBitmap->Release();
		textLayout->Release();
		return entry;
	}

	// 5. テキスト描画（白色で描画、色はSpriteシェーダーで付ける）
	ID2D1SolidColorBrush* brush = nullptr;
	rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush);
	assert(SUCCEEDED(hr) && brush);

	rt->BeginDraw();
	rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));  // 透明クリア
	rt->DrawTextLayout(D2D1::Point2F(1.0f, 1.0f), textLayout, brush);
	hr = rt->EndDraw();

	SAFE_RELEASE(brush);
	SAFE_RELEASE(rt);
	textLayout->Release();

	if (FAILED(hr))
	{
		Logger::Error("Font: D2D EndDraw failed");
		wicBitmap->Release();
		return entry;
	}

	// 6. ピクセルデータを読み取り
	IWICBitmapLock* lock = nullptr;
	WICRect lockRect = { 0, 0, static_cast<INT>(bitmapWidth), static_cast<INT>(bitmapHeight) };
	hr = wicBitmap->Lock(&lockRect, WICBitmapLockRead, &lock);
	if (FAILED(hr))
	{
		Logger::Error("Font: WIC bitmap lock failed");
		wicBitmap->Release();
		return entry;
	}

	UINT bufferSize = 0;
	BYTE* pixelData = nullptr;
	lock->GetDataPointer(&bufferSize, &pixelData);

	UINT stride = 0;
	lock->GetStride(&stride);

	// 7. DX11テクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = bitmapWidth;
	texDesc.Height = bitmapHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixelData;
	initData.SysMemPitch = stride;

	ID3D11Texture2D* texture = nullptr;
	hr = Renderer::GetDevice()->CreateTexture2D(&texDesc, &initData, &texture);

	lock->Release();
	wicBitmap->Release();

	if (FAILED(hr))
	{
		Logger::Error("Font: CreateTexture2D failed");
		return entry;
	}

	// 8. SRV作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = Renderer::GetDevice()->CreateShaderResourceView(texture, &srvDesc, &entry.srv);
	texture->Release();

	if (FAILED(hr))
	{
		Logger::Error("Font: CreateShaderResourceView failed");
		entry.srv = nullptr;
		return entry;
	}

	return entry;
}
