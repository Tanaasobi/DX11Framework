#pragma once

//==============================================================================
// ModelRenderer.h - モデル描画コンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "Model.h"
#include "Shader/IShader.h"

//==============================================================================
// ModelRenderer クラス
// - 3Dモデルを描画するコンポーネント
//==============================================================================
class ModelRenderer : public Component
{
public:
	ModelRenderer();
	virtual ~ModelRenderer();

	void Render() override;

	// モデル設定
	void SetModel(Model* model);
	Model* GetModel() const { return m_Model; }

	// シェーダー設定
	void SetShader(IShader* shader);
	IShader* GetShader() const { return m_Shader; }

private:
	Model* m_Model = nullptr;
	IShader* m_Shader = nullptr;
};
