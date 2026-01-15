#pragma once

//==============================================================================
// TestScene.h - テスト用シーン
//==============================================================================

#include "Core/Scene/Scene.h"
#include "Core/Graphics/Shader/ToonShader.h"
#include "Core/Graphics/SkinnedModel.h"
#include "Core/Animation/AnimationClip.h"
#include "Core/Animation/Animator.h"
#include "Core/Object/Camera.h"

class TestScene : public Scene
{
public:
	TestScene();
	virtual ~TestScene();

	void Init() override;
	void Uninit() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	// トゥーンシェーダー
	ToonShader* m_ToonShader = nullptr;

	// モデル
	SkinnedModel* m_CharacterModel = nullptr;

	// アニメーション
	AnimationClip* m_IdleAnim = nullptr;

	// ゲームオブジェクト
	GameObject* m_CameraObject = nullptr;
	GameObject* m_Character = nullptr;

	// トゥーン設定
	TOON_SETTINGS m_ToonSettings;
	OUTLINE_SETTINGS m_OutlineSettings;
};
