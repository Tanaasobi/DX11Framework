#pragma once

//==============================================================================
// main.h - 共通ヘッダ
//==============================================================================

// Windows
#define NOMINMAX
#include <Windows.h>

// C++ Standard Library
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <iostream>

// DirectX 11
#include <d3d11.h>
#include <d3dcompiler.h>

// DirectXMath
#include <DirectXMath.h>

// リンクするライブラリ
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

//==============================================================================
// 定数定義
//==============================================================================

// 画面解像度
constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;
constexpr bool BORDERLESS_FULLSCREEN = true;

// ウィンドウクラス名・タイトル
constexpr const wchar_t* WINDOW_CLASS_NAME = L"DX11FrameworkClass";
constexpr const wchar_t* WINDOW_TITLE      = L"DX11 Game Framework";

//==============================================================================
// 頂点構造体
//==============================================================================
struct VERTEX_3D
{
	DirectX::XMFLOAT3 Position; // 座標
	DirectX::XMFLOAT3 Normal;   // 法線
	DirectX::XMFLOAT4 Diffuse;  // 頂点カラー
	DirectX::XMFLOAT2 TexCoord;  // テクスチャ座標
};

//==============================================================================
// スキニング用頂点構造体
//==============================================================================
struct VERTEX_3D_SKINNED
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT2 TexCoord;
	UINT              BoneIndices[4];  // 影響を受けるボーンのインデックス
	float             BoneWeights[4];  // 各ボーンの重み
};

//==============================================================================
// マテリアル構造体
//==============================================================================
struct MATERIAL
{
	DirectX::XMFLOAT4 Ambient;  // 環境光
	DirectX::XMFLOAT4 Diffuse;  // 拡散反射光
	DirectX::XMFLOAT4 Specular; // 鏡面反射光
	DirectX::XMFLOAT4 Emission; // 自己発光
	float Shininess;            // 光沢度
	BOOL  TextureEnable;        // テクスチャ有効フラグ
	float Padding[2];
};

//==============================================================================
// ライト構造体（シンプル版）
//==============================================================================
struct LIGHT
{
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Ambient;
};

//==============================================================================
// ユーティリティマクロ
//==============================================================================

// COMオブジェクトの安全な解放
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = nullptr; } }
