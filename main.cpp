//==============================================================================
// main.cpp - アプリケーションエントリポイント
//==============================================================================

#include "Core/System/main.h"
#include "Core/System/Time.h"
#include "Core/System/Input.h"
#include "Core/System/Logger.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Sprite.h"
#include "Core/Graphics/UI/TextRenderer.h"
#include "Core/Scene/SceneManager.h"
#include "Core/Audio/Audio.h"
#include "Game/GameScene.h"
#include "Game/TitleScene.h"
#include "Game/ResultScene.h"

//==============================================================================
// グローバル変数
//==============================================================================
HWND g_hWnd = nullptr;
bool g_Running = true;

//==============================================================================
// ウィンドウプロシージャ
//==============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		g_Running = false;
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
			g_Running = false;
		}
		return 0;

	case WM_MOUSEWHEEL:
	{
		float delta = GET_WHEEL_DELTA_WPARAM(wParam) / static_cast<float>(WHEEL_DELTA);
		Input::SetMouseWheel(delta);
		return 0;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

//==============================================================================
// ウィンドウ作成
//==============================================================================
bool InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszClassName = WINDOW_CLASS_NAME;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(nullptr, L"RegisterClassEx failed", L"Error", MB_OK);
		return false;
	}

	int windowWidth = SCREEN_WIDTH;
	int windowHeight = SCREEN_HEIGHT;
	int posX = 0;
	int posY = 0;
	DWORD style;
	DWORD exStyle = 0;

	if (BORDERLESS_FULLSCREEN)
	{
		// ボーダーレスフルスクリーン
		style = WS_POPUP;
		posX = 0;
		posY = 0;
	}
	else
	{
		// 通常ウィンドウ
		style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

		RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		AdjustWindowRect(&rect, style, FALSE);
		windowWidth = rect.right - rect.left;
		windowHeight = rect.bottom - rect.top;

		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		posX = (screenWidth - windowWidth) / 2;
		posY = (screenHeight - windowHeight) / 2;
	}

	g_hWnd = CreateWindowEx(
		exStyle,
		WINDOW_CLASS_NAME,
		WINDOW_TITLE,
		style,
		posX, posY,
		windowWidth, windowHeight,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (!g_hWnd)
	{
		MessageBox(nullptr, L"CreateWindowEx failed", L"Error", MB_OK);
		return false;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	return true;
}

//==============================================================================
// WinMain
//==============================================================================
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//--------------------------------------------------------------------------
	// 初期化
	//--------------------------------------------------------------------------
	// window初期化
	if (!InitWindow(hInstance, nCmdShow))
	{
		return -1;
	}

	// 描画初期化
	if (!Renderer::Init(g_hWnd))
	{
		Logger::Error("Renderer initialization failed");
		return -1;
	}
	Logger::Info("Renderer initialized");

	// 時間初期化
	Time::Init();
	Logger::Info("Time initialized");

	// 入力初期化
	Input::Init(g_hWnd);
	Logger::Info("Input initialized");

	// テクスチャ初期化
	Texture::Init();
	Logger::Info("Texture initialized");

	// スプライト初期化
	Sprite::Init();
	Logger::Info("Sprite system initialized");

	// テキストレンダラー初期化
	if (!TextRenderer::Init())
	{
		Logger::Error("TextRenderer initialization failed");
		return -1;
	}
	Logger::Info("TextRenderer initialized");

	// Audio初期化
	Audio::InitMaster();
	Logger::Info("Audio initialized");

	// SceneManager初期化
	SceneManager::Init();
	Logger::Info("SceneManager initialized");

	// シーンを登録
	SceneManager::RegisterScene<GameScene>("GameScene");
	SceneManager::RegisterScene<TitleScene>("TitleScene");
	SceneManager::RegisterScene<ResultScene>("ResultScene");

	// 初期シーンをロード
	SceneManager::LoadScene("TitleScene");

	Logger::Info("Game started!");

	//--------------------------------------------------------------------------
	// メインループ
	//--------------------------------------------------------------------------
	MSG msg = {};
	while (g_Running)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				g_Running = false;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// 時間更新
			Time::Update();

			// 入力更新
			Input::Update();

			// シーン更新
			SceneManager::Update(Time::GetDeltaTime());
			SceneManager::LateUpdate(Time::GetDeltaTime());

			// 描画
			Renderer::Begin();
			SceneManager::Render();
			Renderer::End();
		}
	}

	//--------------------------------------------------------------------------
	// 終了処理
	//--------------------------------------------------------------------------
	Logger::Info("Game shutting down...");

	SceneManager::Uninit();
	Audio::UninitMaster();
	TextRenderer::Uninit();
	Sprite::Uninit();
	Texture::Uninit();
	Input::Uninit();
	Renderer::Uninit();

	Logger::Info("Game ended");

	return (int)msg.wParam;
}
