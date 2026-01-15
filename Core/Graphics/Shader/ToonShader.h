#pragma once

//==============================================================================
// ToonShader.h - トゥーンシェーダー
//==============================================================================

#include "IShader.h"
#include "Core/System/main.h"
#include <string>

//==============================================================================
// トゥーン設定構造体
//==============================================================================
struct TOON_SETTINGS
{
	int               Levels;
	float             Edge;
	float             RimPower;
	float             RimIntensity;
	DirectX::XMFLOAT4 RimColor;
};

//==============================================================================
// アウトライン設定構造体
//==============================================================================
struct OUTLINE_SETTINGS
{
	DirectX::XMFLOAT4 Color;
	float             Width;
	DirectX::XMFLOAT3 Padding;
};

//==============================================================================
// ToonShader クラス
// - トゥーンレンダリング用シェーダー
// - メインパス + アウトラインパス
//==============================================================================
class ToonShader : public IShader
{
public:
	ToonShader();
	virtual ~ToonShader();

	// 通常モデル用
	bool Load(const std::wstring& vsFile, const std::wstring& psFile);

	// スキニングモデル用
	bool LoadSkinned(const std::wstring& vsFile, const std::wstring& psFile);

	// アウトライン用
	bool LoadOutline(const std::wstring& vsFile, const std::wstring& psFile);
	bool LoadSkinnedOutline(const std::wstring& vsFile, const std::wstring& psFile);

	// シェーダーをセット（メイン）
	void Set() override;

	// アウトラインシェーダーをセット
	void SetOutline();

	// トゥーン設定
	void SetToonSettings(const TOON_SETTINGS& settings);

	// アウトライン設定
	void SetOutlineSettings(const OUTLINE_SETTINGS& settings);

	// アウトラインを描画するか
	bool enableOutline = true;

	// スキニング対応か
	bool IsSkinned() const { return m_IsSkinned; }

private:
	// メインシェーダー
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;

	// アウトラインシェーダー
	ID3D11VertexShader* m_OutlineVertexShader = nullptr;
	ID3D11PixelShader* m_OutlinePixelShader = nullptr;
	ID3D11InputLayout* m_OutlineInputLayout = nullptr;

	// 定数バッファ
	ID3D11Buffer* m_ToonBuffer = nullptr;
	ID3D11Buffer* m_OutlineBuffer = nullptr;

	bool m_IsSkinned = false;

	bool CreateConstantBuffers();
	bool CreateInputLayout(ID3DBlob* vsBlob, bool skinned);
};
