//==============================================================================
// Player.cpp - プレイヤー実装
//==============================================================================

#include "Player.h"
#include "Field.h"
#include "Core/Graphics/SkinnedMeshRenderer.h"
#include "Core/Physics/Collider.h"
#include "Core/System/Input.h"
#include "Core/System/Logger.h"
#include "Core/Object/Camera.h"
#include <cmath>
#include <algorithm>

//==============================================================================
// コンストラクタ
//==============================================================================
Player::Player()
	: GameObject("Player")
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Player::~Player()
{
	if (m_Model)
	{
		delete m_Model;
		m_Model = nullptr;
	}
	if (m_IdleAnim)
	{
		delete m_IdleAnim;
		m_IdleAnim = nullptr;
	}
	if (m_WalkAnim)
	{
		delete m_WalkAnim;
		m_WalkAnim = nullptr;
	}
	if (m_KickAnim)
	{
		delete m_KickAnim;
		m_KickAnim = nullptr;
	}
}

//==============================================================================
// 初期化
//==============================================================================
void Player::Init(IShader* shader)
{
	//--------------------------------------------------------------------------
	// モデル読み込み
	//--------------------------------------------------------------------------
	m_Model = new SkinnedModel();
	if (!m_Model->Load("Asset/Model/character_base.fbx"))
	{
		Logger::Error("Failed to load player model!");
	}

	//--------------------------------------------------------------------------
	// アニメーション読み込み
	//--------------------------------------------------------------------------
	m_IdleAnim = new AnimationClip();
	if (!m_IdleAnim->Load("Asset/Anim/idle.fbx"))
	{
		Logger::Error("Failed to load idle animation!");
	}

	m_WalkAnim = new AnimationClip();
	if (!m_WalkAnim->Load("Asset/Anim/walk.fbx"))
	{
		Logger::Error("Failed to load walk animation!");
	}

	m_KickAnim = new AnimationClip();
	if (m_KickAnim->Load("Asset/Anim/kick.fbx"))
	{
		Logger::InfoFormat("Kick animation loaded: duration=%.2f", m_KickAnim->GetDuration());
	}

	//--------------------------------------------------------------------------
	// Animator追加
	//--------------------------------------------------------------------------
	m_Animator = AddComponent<Animator>();
	m_Animator->SetSkeleton(m_Model->GetSkeleton());
	m_Animator->Play(m_IdleAnim);

	//--------------------------------------------------------------------------
	// SkinnedMeshRenderer追加
	//--------------------------------------------------------------------------
	SkinnedMeshRenderer* renderer = AddComponent<SkinnedMeshRenderer>();
	renderer->SetModel(m_Model);
	renderer->SetShader(shader);
	renderer->SetAnimator(m_Animator);

	//--------------------------------------------------------------------------
	// コライダー追加
	//--------------------------------------------------------------------------
	m_Collider = AddComponent<CircleCollider>();
	m_Collider->radius = 0.5f;

	//--------------------------------------------------------------------------
	// 初期位置・スケール
	//--------------------------------------------------------------------------
	GetTransform()->position = Vector3(-8.0f, 0.0f, 0.0f);
	GetTransform()->scale = Vector3(0.032f, 0.032f, 0.032f);

	m_CurrentRotationY = 0.0f;
	GetTransform()->SetEulerAngles(0.0f, m_CurrentRotationY, 0.0f);

	Logger::Info("Player initialized with animations");
}

//==============================================================================
// 更新
//==============================================================================
void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	// クールダウン更新
	if (m_KickCooldownTimer > 0.0f)
	{
		m_KickCooldownTimer -= deltaTime;
	}

	// 入力が無効なら移動しない
	if (!m_InputEnabled)
	{
		m_IsMoving = false;

		// アニメーションをアイドルに
		if (m_WasMoving)
		{
			m_Animator->CrossFade(m_IdleAnim, 0.2f);
			m_WasMoving = false;
		}
		return;
	}

	// 入力取得（以下は既存のコード）
	float moveX = 0.0f;
	float moveZ = 0.0f;

	if (m_UseExternalInput)
	{
		moveX = m_MoveInputX;
		moveZ = m_MoveInputZ;
	}
	else
	{
		if (Input::GetKey(KeyCode::W)) moveZ += 1.0f;
		if (Input::GetKey(KeyCode::S)) moveZ -= 1.0f;
		if (Input::GetKey(KeyCode::A)) moveX -= 1.0f;
		if (Input::GetKey(KeyCode::D)) moveX += 1.0f;

		if (Input::IsGamepadConnected(0))
		{
			float sx, sy;
			Input::GetGamepadLeftStick(0, sx, sy);
			moveX += sx;
			moveZ += sy;

			if (Input::GetGamepadButton(0, GamepadButton::DPadUp))    moveZ += 1.0f;
			if (Input::GetGamepadButton(0, GamepadButton::DPadDown))  moveZ -= 1.0f;
			if (Input::GetGamepadButton(0, GamepadButton::DPadLeft))  moveX -= 1.0f;
			if (Input::GetGamepadButton(0, GamepadButton::DPadRight)) moveX += 1.0f;
		}
	}


	// 正規化
	float length = std::sqrt(moveX * moveX + moveZ * moveZ);

	// 倒し量（0〜1）
	float magnitude = (length > 0.0f) ? std::min(length, 1.0f) : 0.0f;
	m_IsMoving = (length > 0.0f);

	float dirX = 0.0f;
	float dirZ = 0.0f;

	if (length > 0.0001f)
	{
		dirX = moveX / length;
		dirZ = moveZ / length;

		UpdateRotation(dirX, dirZ, deltaTime);
	}

	// 移動適用
	Transform* transform = GetTransform();
	transform->position.x += dirX * moveSpeed * magnitude * deltaTime;
	transform->position.z += dirZ * moveSpeed * magnitude * deltaTime;

	ClampToField();

	//--------------------------------------------------------------------------
	// アニメーション切り替え（歩き/待機）
	// キックアニメーション再生中でなければ切り替え
	//--------------------------------------------------------------------------
	if (!m_Animator->IsPlaying(m_KickAnim))
	{
		if (m_IsMoving != m_WasMoving)
		{
			if (m_IsMoving)
			{
				m_Animator->CrossFade(m_WalkAnim, 0.2f);
			}
			else
			{
				m_Animator->CrossFade(m_IdleAnim, 0.2f);
			}
			m_WasMoving = m_IsMoving;
		}
	}
}

//==============================================================================
// キック可能か
//==============================================================================
bool Player::CanKick() const
{
	return m_InputEnabled && m_KickCooldownTimer <= 0.0f;
}
//==============================================================================
// キック実行
//==============================================================================
bool Player::TryKick()
{
	if (!CanKick()) return false;

	m_KickCooldownTimer = kickCooldown;

	// キックアニメーション再生（最大0.5秒で再生）
	m_Animator->PlayOnce(m_KickAnim, 0.1f, 0.5f);

	return true;
}

//==============================================================================
// 移動方向に向きを変える
//==============================================================================
void Player::UpdateRotation(float dirX, float dirZ, float deltaTime)
{
	// 移動方向からY軸回転角度を計算
	// atan2(x, z) で Z+方向が0度、X+方向が90度
	float targetAngle = std::atan2(dirX, dirZ);
	float targetDegrees = targetAngle * (180.0f / 3.14159265f);
	targetDegrees += 180.0f;

	// 角度差を計算
	float angleDiff = targetDegrees - m_CurrentRotationY;

	// -180〜180の範囲に正規化
	while (angleDiff > 180.0f) angleDiff -= 360.0f;
	while (angleDiff < -180.0f) angleDiff += 360.0f;

	// スムーズに回転
	float rotationSpeed = 540.0f;  // 度/秒
	float maxRotation = rotationSpeed * deltaTime;

	if (angleDiff > maxRotation) angleDiff = maxRotation;
	if (angleDiff < -maxRotation) angleDiff = -maxRotation;

	m_CurrentRotationY += angleDiff;

	// -180〜180の範囲に正規化
	while (m_CurrentRotationY > 180.0f) m_CurrentRotationY -= 360.0f;
	while (m_CurrentRotationY < -180.0f) m_CurrentRotationY += 360.0f;

	// Y軸回転のみを適用（X, Zは0に固定）
	GetTransform()->SetEulerAngles(0.0f, m_CurrentRotationY, 0.0f);
}

//==============================================================================
// フィールド内に制限
//==============================================================================
void Player::ClampToField()
{
	Transform* transform = GetTransform();
	float halfSize = m_Collider->radius;

	transform->position.x = std::clamp(transform->position.x,
		FieldBounds::LEFT + halfSize, FieldBounds::RIGHT - halfSize);
	transform->position.z = std::clamp(transform->position.z,
		FieldBounds::TOP + halfSize, FieldBounds::BOTTOM - halfSize);
}

//==============================================================================
// 蹴る方向を取得
//==============================================================================
void Player::GetKickDirection(float& outDirX, float& outDirZ)
{
	//--------------------------------------------------------------------------
	// ゲームパッド（右スティック優先）
	//--------------------------------------------------------------------------
	if (Input::IsGamepadConnected(0))
	{
		float rx, ry;
		Input::GetGamepadRightStick(0, rx, ry);

		float len = std::sqrt(rx * rx + ry * ry);
		if (len > 0.2f)
		{
			outDirX = rx / len;
			outDirZ = ry / len;
			return;
		}

		// 右スティック入力が無ければ「現在向いている方向」に蹴る
		constexpr float PI = 3.14159265f;
		float rad = m_CurrentRotationY * (PI / 180.0f);
		outDirX = -std::sinf(rad);
		outDirZ = -std::cosf(rad);
		return;
	}


	int screenX, screenY;
	Input::GetMousePosition(screenX, screenY);

	Camera* camera = Camera::GetMain();
	if (!camera)
	{
		outDirX = 0.0f;
		outDirZ = 1.0f;
		return;
	}

	Vector3 rayOrigin, rayDir;
	camera->ScreenToWorldRay(screenX, screenY, rayOrigin, rayDir);

	if (std::abs(rayDir.y) < 0.0001f)
	{
		outDirX = 0.0f;
		outDirZ = 1.0f;
		return;
	}

	float t = -rayOrigin.y / rayDir.y;
	if (t < 0.0f)
	{
		outDirX = 0.0f;
		outDirZ = 1.0f;
		return;
	}

	Vector3 mouseWorld;
	mouseWorld.x = rayOrigin.x + rayDir.x * t;
	mouseWorld.z = rayOrigin.z + rayDir.z * t;

	Transform* transform = GetTransform();
	float dx = mouseWorld.x - transform->position.x;
	float dz = mouseWorld.z - transform->position.z;

	float len = std::sqrt(dx * dx + dz * dz);
	if (len > 0.0f)
	{
		outDirX = dx / len;
		outDirZ = dz / len;
	}
	else
	{
		outDirX = 0.0f;
		outDirZ = 1.0f;
	}
}

//==============================================================================
// 外部入力
//==============================================================================
void Player::EnableExternalInput(bool enable)
{
	m_UseExternalInput = enable;
}

void Player::SetMoveInput(float x, float z)
{
	m_MoveInputX = x;
	m_MoveInputZ = z;
}
