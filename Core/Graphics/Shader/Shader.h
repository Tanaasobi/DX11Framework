#pragma once

//==============================================================================
// Shader.h - 基本シェーダー
//==============================================================================

#include "IShader.h"
#include "Core/System/main.h"
#include <string>

//==============================================================================
// Shader クラス
// - 通常の3Dモデル用シェーダー
//==============================================================================
class Shader : public IShader
{
public:
	Shader();
	virtual ~Shader();

	// 読み込み
	bool Load(const std::wstring& vsFile, const std::wstring& psFile);

	// シェーダーをセット
	void Set() override;

protected:
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;

	// 派生クラス用：入力レイアウト作成
	virtual bool CreateInputLayout(ID3DBlob* vsBlob);
};
