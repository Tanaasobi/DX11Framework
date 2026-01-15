#pragma once

//==============================================================================
// SkinnedMeshRenderer.h - スキンメッシュ描画コンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "SkinnedModel.h"
#include "Shader/IShader.h"
#include "Core/Animation/Animator.h"

//==============================================================================
// SkinnedMeshRenderer クラス
//==============================================================================
class SkinnedMeshRenderer : public Component
{
public:
	SkinnedMeshRenderer();
	virtual ~SkinnedMeshRenderer();

	void Render() override;

	void SetModel(SkinnedModel* model);
	void SetShader(IShader* shader);
	void SetAnimator(Animator* animator);

	SkinnedModel* GetModel() const { return m_Model; }

private:
	SkinnedModel* m_Model = nullptr;
	IShader* m_Shader = nullptr;
	Animator* m_Animator = nullptr;
};
