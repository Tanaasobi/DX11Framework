//==============================================================================
// CpuAI.cpp - CPU AI実装
//==============================================================================

#include "CpuAI.h"
#include "Player.h"
#include "Puck.h"
#include "Field.h"
#include "Core/System/Logger.h"
#include "Core/Physics/Collider.h"
#include <cmath>

CpuAI::CpuAI()
	: m_Rng(std::random_device{}())
	, m_KickIntervalRand(0.7f, 1.3f)
{
}

CpuAI::~CpuAI()
{
}

void CpuAI::Init(Player* ownerPlayer, Puck* targetPuck)
{
	m_Owner = ownerPlayer;
	m_TargetPuck = targetPuck;

	// オーナー（Player）の外部入力を有効化しておく
	if (m_Owner)
	{
		m_Owner->EnableExternalInput(true);
	}
}

void CpuAI::Update(float deltaTime)
{
	if (!m_IsActive || !m_Owner || !m_TargetPuck) return;

	// プレイヤー自体の入力が無効ならAIも動かない
	if (!m_Owner->IsInputEnabled()) return;

	UpdateState();
	ExecuteAction(deltaTime);
}

void CpuAI::UpdateState()
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	Vector3 myPos = m_Owner->GetTransform()->position;

	// パックが自分より後ろ（ゴール側）なら守備
	if (puckPos.x > myPos.x)
	{
		m_State = CpuState::Defend;
		return;
	}

	// 自陣深くなら攻撃
	if (puckPos.x > 0.0f)
	{
		m_State = CpuState::Attack;
	}
	else
	{
		m_State = CpuState::Defend;
	}

	// 距離チェック（簡易計算）
	float dx = puckPos.x - myPos.x;
	float dz = puckPos.z - myPos.z;
	float distSq = dx * dx + dz * dz;

	// 射程圏内かつクールダウン完了ならシュート
	if (distSq < 2.5f * 2.5f && m_KickTimer <= 0.0f)
	{
		m_State = CpuState::Shoot;
	}
}

void CpuAI::ExecuteAction(float deltaTime)
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	Vector3 targetPos = m_Owner->GetTransform()->position;

	switch (m_State)
	{
	case CpuState::Defend:
		// 守備位置へ：Xは定位置、Zはパックに合わせる
		targetPos.x = HOME_POS_X;
		targetPos.z = puckPos.z * 0.5f;
		MoveToTarget(targetPos);
		break;

	case CpuState::Attack:
		// 攻撃：パックを追う
		MoveToTarget(puckPos);
		break;

	case CpuState::Shoot:
		MoveToTarget(puckPos);

		m_KickTimer -= deltaTime;
		if (m_KickTimer <= 0.0f)
		{
			// キック実行
			float dirX, dirZ;
			GetShootDirection(dirX, dirZ);

			// Playerクラスのキック判定を利用
			// ※Player::TryKickは引数なし版しかないので、
			// 向き指定で蹴るにはロジック修正が必要かもしれないが、
			// いったん「移動入力方向に蹴る」仕様ならこれでOK。
			// もし「移動方向と違う方向に蹴りたい」ならPlayerクラスにメソッド追加が必要。

			// ここでは簡易的に「Playerをその方向に向かせて蹴る」
			// もしくはGameSceneにあったTryKickPuckロジックを移植する必要がある
			// 今回はPlayerに「方向指定キック」があると仮定、あるいは
			// 移動入力で向きが変わるので、その向きに蹴る

			// 確実に方向指定で蹴らせるため、一時的にPlayerを拡張するか、
			// ここでパックの速度を直接いじる（GameSceneのロジックと同じなら）

			// ★ここだけはPlayer.hへの依存が強い部分です
			// 今回は「AIがパックを操作する」のではなく「AIがPlayerを操作する」形にします

			if (m_Owner->CanKick()) // クールダウンチェック
			{
				// 距離チェックなど簡易判定
				float dx = m_Owner->GetCollider()->GetX() - m_TargetPuck->GetCollider()->GetX();
				float dy = m_Owner->GetCollider()->GetY() - m_TargetPuck->GetCollider()->GetY();
				float dist = std::sqrt(dx * dx + dy * dy);

				if (dist < 3.0f)
				{
					if (m_Owner->TryKick())
					{
						// パックを弾く
						float kickPower = 25.0f;
						m_TargetPuck->SetVelocity(dirX * kickPower, dirZ * kickPower);

						Logger::Info("CpuAI Kicked!");
						m_KickTimer = m_KickIntervalRand(m_Rng);
						m_State = CpuState::Defend;
					}
				}
			}
		}
		break;

	default:
		m_Owner->SetMoveInput(0.0f, 0.0f);
		break;
	}
}

void CpuAI::MoveToTarget(const Vector3& targetPos)
{
	Vector3 myPos = m_Owner->GetTransform()->position;
	float dx = targetPos.x - myPos.x;
	float dz = targetPos.z - myPos.z;
	float dist = std::sqrt(dx * dx + dz * dz);

	if (dist > 0.1f)
	{
		m_Owner->SetMoveInput(dx / dist, dz / dist);
	}
	else
	{
		m_Owner->SetMoveInput(0.0f, 0.0f);
	}
}

void CpuAI::GetShootDirection(float& outDirX, float& outDirZ)
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	// 相手ゴール（左）の中心
	float targetX = FieldBounds::LEFT;
	float targetZ = 0.0f;

	float dx = targetX - puckPos.x;
	float dz = targetZ - puckPos.z;
	float len = std::sqrt(dx * dx + dz * dz);

	if (len > 0.0f)
	{
		outDirX = dx / len;
		outDirZ = dz / len;
	}
	else
	{
		outDirX = -1.0f;
		outDirZ = 0.0f;
	}
}
