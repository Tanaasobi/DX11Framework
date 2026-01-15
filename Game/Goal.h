#pragma once

//==============================================================================
// Goal.h - ゴール
//==============================================================================

#include "Core/Object/GameObject.h"
#include "Core/Graphics/Shader/IShader.h"
#include "Core/Physics/Collider.h"

//==============================================================================
// ゴールのチーム
//==============================================================================
enum class Team
{
	Left,   // 左側（チーム1）
	Right   // 右側（チーム2）
};

//==============================================================================
// Goal クラス
//==============================================================================
class Goal : public GameObject
{
public:
	Goal(Team team);
	virtual ~Goal();

	void Init(IShader* shader);
	void Update(float deltaTime) override;

	// コライダー取得
	BoxCollider* GetCollider() const { return m_Collider; }

	// チーム取得
	Team GetTeam() const { return m_Team; }

	// ゴールサイズ
	static constexpr float WIDTH = 1.5f;
	static constexpr float HEIGHT = 8.0f;
	static constexpr float DEPTH = 0.8f;

private:
	Team m_Team;
	BoxCollider* m_Collider = nullptr;
};
