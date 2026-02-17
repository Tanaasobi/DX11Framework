#pragma once

//==============================================================================
// Player.h - プレイヤー
//==============================================================================

#include "Core/Object/GameObject.h"
#include "Core/Graphics/Shader/IShader.h"
#include "Core/Graphics/SkinnedModel.h"
#include "Core/Animation/AnimationClip.h"
#include "Core/Animation/Animator.h"

class CircleCollider;
class SkinnedMeshRenderer;
class Puck;
class RingGauge;

class Player : public GameObject
{
public:
	Player();
	virtual ~Player();

	void Init(IShader* shader);
	void Update(float deltaTime) override;

	CircleCollider* GetCollider() const { return m_Collider; }

	// キック実行（プレイヤー操作用）
	bool Kick(Puck* targetPuck);

	// キック実行(AI用)
	bool Kick(Puck* targetPuck, const Vector3& dir, float powerOverride = -1.0f);

	// キック可能か
	bool CanKick() const;

	// チャージ操作
	void StartCharge();
	bool ReleaseKick(Puck* targetPuck);

	// パラメータ
	float moveSpeed = 12.0f;
	float kickCooldown = 0.5f;
	float kickPower = 25.0f;
	float kickRange = 1.7f;

	// チャージ用パラメータ
	float maxKickPower = 50.0f; // 最大チャージ時のパワー
	float maxChargeTime = 1.0f; // 最大パワーまでの時間（秒）

	// 入力を有効/無効にする
	void SetInputEnabled(bool enabled) { m_InputEnabled = enabled; }
	bool IsInputEnabled() const { return m_InputEnabled; }

	//--------------------------------------------------------------------------
	// 外部入力（CPU等）
	//--------------------------------------------------------------------------
	void EnableExternalInput(bool enable);
	void SetMoveInput(float x, float z);

private:
	CircleCollider* m_Collider = nullptr;

	// アニメーション関連
	SkinnedModel* m_Model = nullptr;
	AnimationClip* m_IdleAnim = nullptr;
	AnimationClip* m_WalkAnim = nullptr;
	AnimationClip* m_KickAnim = nullptr;
	Animator* m_Animator = nullptr;

	// 状態
	bool  m_IsMoving = false;
	bool  m_WasMoving = false;
	float m_CurrentRotationY = 0.0f;
	bool m_InputEnabled = true;

	// チャージゲージUI
	RingGauge* m_ChargeGauge = nullptr;

	// チャージ状態
	bool m_IsCharging = false;
	float m_ChargeTimer = 0.0f;

	// キック状態
	float m_KickCooldownTimer = 0.0f;

	void ClampToField();
	void UpdateRotation(float dirX, float dirZ, float deltaTime);

	// 外部入力
	bool  m_UseExternalInput = false;
	float m_MoveInputX = 0.0f;
	float m_MoveInputZ = 0.0f;
};
