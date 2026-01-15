//==============================================================================
// SkinnedMeshRenderer.cpp - スキンメッシュ描画コンポーネント実装
//==============================================================================

#include "SkinnedMeshRenderer.h"
#include "Renderer.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"

//==============================================================================
// コンストラクタ
//==============================================================================
SkinnedMeshRenderer::SkinnedMeshRenderer()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
SkinnedMeshRenderer::~SkinnedMeshRenderer()
{
}

//==============================================================================
// 描画
//==============================================================================
void SkinnedMeshRenderer::Render()
{
	if (!m_Model || !m_Shader) return;

	// シェーダー設定
	m_Shader->Set();

	// ワールド行列設定
	Transform* transform = GetGameObject()->GetTransform();
	Renderer::SetWorldMatrix(transform->GetWorldMatrix());

	// ボーン行列設定
	if (m_Animator)
	{
		const auto& boneMatrices = m_Animator->GetBoneMatrices();
		if (!boneMatrices.empty())
		{
			Renderer::SetBoneMatrices(boneMatrices.data(), static_cast<int>(boneMatrices.size()));
		}
	}

	// モデル描画
	m_Model->Draw();
}

//==============================================================================
// モデル設定
//==============================================================================
void SkinnedMeshRenderer::SetModel(SkinnedModel* model)
{
	m_Model = model;
}

//==============================================================================
// シェーダー設定
//==============================================================================
void SkinnedMeshRenderer::SetShader(IShader* shader)
{
	m_Shader = shader;
}

//==============================================================================
// アニメーター設定
//==============================================================================
void SkinnedMeshRenderer::SetAnimator(Animator* animator)
{
	m_Animator = animator;
}
