//==============================================================================
// CpuAI.cpp - 強化版CPUロジック実装
//==============================================================================

#include "CpuAI.h"
#include "Player.h"
#include "Puck.h"
#include "Field.h"
#include "Core/Physics/Collider.h"
#include "Core/System/Logger.h"
#include "Core/Scene/SceneManager.h"
#include "Core/Scene/Scene.h" // Sceneクラスの定義が必要
#include <cmath>
#include <algorithm>

using namespace DirectX;

CpuAI::CpuAI()
	: m_Rng(std::random_device{}())
{
}

CpuAI::~CpuAI()
{
}

void CpuAI::Init(Player* ownerPlayer, Puck* targetPuck)
{
	m_Owner = ownerPlayer;
	m_TargetPuck = targetPuck;

	// 相手プレイヤーを探す
	// FindGameObjectOfTypeは最初に見つかった1つを返す仕様と想定（通常は生成順）
	// Player(人間)はCPUより先に生成されているため、これで取得できるはず
	Scene* activeScene = SceneManager::GetActiveScene();
	if (activeScene)
	{
		Player* foundPlayer = activeScene->FindGameObjectOfType<Player>();

		// 見つかったのが自分自身でないなら、それが敵（プレイヤー）
		if (foundPlayer != m_Owner)
		{
			m_EnemyPlayer = foundPlayer;
		}
		else
		{
			// もし自分が見つかってしまった場合、リスト等から検索が必要だが
			// 現状の実装順（Player -> Puck -> CPU）なら問題ないはず
			Logger::Warning("CpuAI: Enemy player not found (found self).");
		}
	}

	if (m_Owner)
	{
		m_Owner->EnableExternalInput(true);
	}
}

void CpuAI::Update(float deltaTime)
{
	if (!m_IsActive || !m_Owner || !m_TargetPuck) return;
	if (!m_Owner->IsInputEnabled()) return;

	// 思考ルーチン（一定間隔で更新して判断を安定させる）
	UpdateDecision(deltaTime);

	// 決定した状態に基づいて行動を実行
	ExecuteAction(deltaTime);
}

//==============================================================================
// 状況判断・ステート遷移
//==============================================================================
void CpuAI::UpdateDecision(float deltaTime)
{
	// シュートやクリアなどのアクション中は状態を変えない
	if (m_State == CpuState::Shoot || m_State == CpuState::Clear) return;

	m_ThinkTimer -= deltaTime;
	if (m_ThinkTimer <= 0.0f)
	{
		m_ThinkTimer = m_ThinkInterval;
		DecideState();
	}
}

void CpuAI::DecideState()
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	Vector3 myPos = m_Owner->GetTransform()->position;
	float puckVelX = 0.0f;
	float puckVelZ = 0.0f;
	m_TargetPuck->GetVelocity(puckVelX, puckVelZ);

	// 1. キック範囲内なら即座にアクション決定
	// Vector3::DistanceSquaredがないため手動計算
	Vector3 diff = myPos - puckPos;
	float distSq = diff.LengthSquared();

	if (distSq < 2.5f * 2.5f && m_Owner->CanKick())
	{
		// 自陣深くならクリア、敵陣寄りならシュート
		if (puckPos.x > 0.0f)
		{
			m_State = CpuState::Clear; // 自陣（右側）なのでクリア
		}
		else
		{
			m_State = CpuState::Shoot; // 敵陣なのでシュート
		}
		m_ActionTimer = 0.2f; // アクション予備動作時間
		return;
	}

	// 2. パックへの到達予測時間を比較
	float myReachTime = CalculateReachTime(myPos, m_Owner->moveSpeed, puckPos);
	float enemyReachTime = 100.0f;

	if (m_EnemyPlayer)
	{
		enemyReachTime = CalculateReachTime(m_EnemyPlayer->GetTransform()->position,
			m_EnemyPlayer->moveSpeed, puckPos);
	}

	// 3. 状況別ステート遷移
	bool isPuckComing = (puckVelX > 0.5f); // パックが自陣（右）に向かっている
	bool isPuckInMySide = (puckPos.x > 0.0f); // パックが自陣にある

	if (isPuckInMySide)
	{
		// 自陣にある場合
		if (myReachTime < enemyReachTime * 0.8f) // 明らかに自分が近い
		{
			m_State = CpuState::Attack;
		}
		else
		{
			// 相手の方が近い、または微妙な距離 -> ゴールを守る
			m_State = CpuState::Defend;
		}
	}
	else
	{
		// 敵陣にある場合
		if (myReachTime < enemyReachTime && !isPuckComing)
		{
			// 自分が近く、かつパックが向かってきていない（止まっている等）なら攻める
			m_State = CpuState::Attack;
		}
		else if (isPuckComing)
		{
			// 敵陣にあるが、こっちに向かってきている -> カウンターに備えて下がる
			m_State = CpuState::Defend;
		}
		else
		{
			// 敵がボールを持っていて、まだ攻めてきていない -> 待機
			m_State = CpuState::Wait;
		}
	}
}

float CpuAI::CalculateReachTime(const Vector3& fromPos, float moveSpeed, const Vector3& targetPos)
{
	if (moveSpeed <= 0.0f) return 999.0f;
	// Vector3::Distanceがないため手動計算
	float dist = (fromPos - targetPos).Length();
	return dist / moveSpeed;
}

//==============================================================================
// 行動実行
//==============================================================================
void CpuAI::ExecuteAction(float deltaTime)
{
	switch (m_State)
	{
	case CpuState::Attack: UpdateAttack(); break;
	case CpuState::Defend: UpdateDefend(); break;
	case CpuState::Wait:   UpdateWait();   break;
	case CpuState::Shoot:  UpdateShoot(deltaTime);  break;
	case CpuState::Clear:  UpdateClear(deltaTime);  break;
	}
}

//------------------------------------------------------------------------------
// 攻め: パックを最短で追う
//------------------------------------------------------------------------------
void CpuAI::UpdateAttack()
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;

	// 少しだけ未来位置を追うとスムーズ
	float vx, vz;
	m_TargetPuck->GetVelocity(vx, vz);
	Vector3 target = puckPos + Vector3(vx, 0, vz) * 0.2f;

	MoveTo(target);
}

//------------------------------------------------------------------------------
// 守り: 先読みして守備位置へ
//------------------------------------------------------------------------------
void CpuAI::UpdateDefend()
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	float vx, vz;
	m_TargetPuck->GetVelocity(vx, vz);

	Vector3 targetPos;
	targetPos.x = DEFEND_LINE_X; // 基本守備ライン

	// パックが向かってきている(Vx > 0)なら、到達地点を予測
	if (vx > 0.1f)
	{
		targetPos = PredictPuckPosOnLine(DEFEND_LINE_X);
	}
	else
	{
		// 向かってきていないなら、パックのZ座標に合わせてスライド
		targetPos.z = puckPos.z;
	}

	// 守備位置に向かうが、X方向はあまり前に出過ぎないようにClamp
	targetPos.x = std::max(targetPos.x, DEFEND_LINE_X);

	MoveTo(targetPos);
}

//------------------------------------------------------------------------------
// 待機: センターライン手前で様子見
//------------------------------------------------------------------------------
void CpuAI::UpdateWait()
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;

	Vector3 targetPos;
	targetPos.x = WAIT_POS_X;
	targetPos.z = puckPos.z * 0.5f; // 相手の動きに合わせて少し左右に動く

	MoveTo(targetPos);
}

//------------------------------------------------------------------------------
// シュート: 確率で打ち分ける
//------------------------------------------------------------------------------
void CpuAI::UpdateShoot(float deltaTime)
{
	// パックに向かって移動
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	MoveTo(puckPos);

	m_ActionTimer -= deltaTime;
	if (m_ActionTimer <= 0.0f)
	{
		// シュート方向決定
		Vector3 shootDir;

		// 確率で打ち分け
		std::uniform_int_distribution<int> dist(0, 99);
		if (dist(m_Rng) < PROB_WALL_SHOT)
		{
			shootDir = GetBounceShotDir();
			Logger::Info("CpuAI: Bounce Shot!");
		}
		else
		{
			shootDir = GetDirectShotDir();
			Logger::Info("CpuAI: Direct Shot!");
		}

		if (TryKick(shootDir))
		{
			// シュート後は少し守備に戻る
			m_State = CpuState::Defend;
			m_ThinkTimer = 0.5f; // 少し硬直
		}
	}
}

//------------------------------------------------------------------------------
// クリア: 敵がいない方へ蹴る
//------------------------------------------------------------------------------
void CpuAI::UpdateClear(float deltaTime)
{
	// パックに向かって移動
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	MoveTo(puckPos);

	m_ActionTimer -= deltaTime;
	if (m_ActionTimer <= 0.0f)
	{
		// クリア方向決定
		Vector3 clearDir;

		std::uniform_int_distribution<int> dist(0, 99);
		if (dist(m_Rng) < PROB_CLEAR_SAFE && m_EnemyPlayer)
		{
			clearDir = GetSafeClearDir();
			Logger::Info("CpuAI: Safe Clear!");
		}
		else
		{
			// とりあえず前（敵ゴール方向）へ強く蹴る
			clearDir = Vector3(-1.0f, 0.0f, (dist(m_Rng) % 2 == 0) ? 0.5f : -0.5f);
			clearDir.Normalize();
		}

		if (TryKick(clearDir))
		{
			m_State = CpuState::Defend;
			m_ThinkTimer = 0.3f;
		}
	}
}

//==============================================================================
// ヘルパー関数
//==============================================================================

void CpuAI::MoveTo(const Vector3& targetPos)
{
	Vector3 myPos = m_Owner->GetTransform()->position;

	// Vector3::Distanceがないため手動計算
	Vector3 diff = targetPos - myPos;
	float dist = diff.Length();

	if (dist > 0.1f)
	{
		m_Owner->SetMoveInput(diff.x / dist, diff.z / dist);
	}
	else
	{
		m_Owner->SetMoveInput(0.0f, 0.0f);
	}
}

bool CpuAI::TryKick(const Vector3& dir)
{
	if (!m_Owner->CanKick()) return false;

	// キック実行（Playerクラスの機能を利用）
	if (m_Owner->TryKick())
	{
		// 本来はPlayerが蹴った方向に飛ぶが、AIの補正として
		// パックの速度を直接上書きして「狙った方向に蹴れた」ことにする
		float kickPower = 25.0f;
		m_TargetPuck->SetVelocity(dir.x * kickPower, dir.z * kickPower);
		return true;
	}
	return false;
}

// パックの先読み（壁反射考慮）
Vector3 CpuAI::PredictPuckPosOnLine(float targetX)
{
	Vector3 p = m_TargetPuck->GetTransform()->position;
	float vx, vz;
	m_TargetPuck->GetVelocity(vx, vz);

	// 速度がほぼ0、またはターゲットと逆向きなら現在位置を返す
	if (std::abs(vx) < 0.01f) return p;
	if ((targetX - p.x) * vx < 0) return p;

	// 到達までの時間
	float t = (targetX - p.x) / vx;

	// 到達時のZ座標（反射なし）
	float predictedZ = p.z + vz * t;

	// フィールドの上下範囲（FieldBounds::TOP/BOTTOM が使えない場合は定数で代用）
	// ここでは Field.h の定義値を想定: Top=-8.5, Bottom=8.5 とする
	// 必要に応じて Field.h をインクルードして FieldBounds::TOP などを使用してください
	const float FIELD_TOP = -8.0f;
	const float FIELD_BOTTOM = 8.0f;

	// 反射計算（簡易版）
	// 範囲を超えている間、折り返す
	while (predictedZ < FIELD_TOP || predictedZ > FIELD_BOTTOM)
	{
		if (predictedZ < FIELD_TOP)
		{
			float over = FIELD_TOP - predictedZ;
			predictedZ = FIELD_TOP + over;
		}
		else if (predictedZ > FIELD_BOTTOM)
		{
			float over = predictedZ - FIELD_BOTTOM;
			predictedZ = FIELD_BOTTOM - over;
		}
	}

	return Vector3(targetX, 0.0f, predictedZ);
}

// 直接シュート方向（相手ゴール中心）
Vector3 CpuAI::GetDirectShotDir()
{
	Vector3 p = m_TargetPuck->GetTransform()->position;
	// 相手ゴール（左）の中心 (-10, 0, 0) 付近
	Vector3 target(-10.0f, 0.0f, 0.0f);

	Vector3 dir = target - p;
	dir.Normalize();
	return dir;
}

// バウンドシュート方向
Vector3 CpuAI::GetBounceShotDir()
{
	// 上の壁か下の壁かをランダムで選ぶ
	std::uniform_int_distribution<int> dist(0, 1);
	bool aimTop = (dist(m_Rng) == 0);

	Vector3 p = m_TargetPuck->GetTransform()->position;

	// 狙う壁のポイント（敵陣深くの壁）
	float targetWallX = -5.0f;
	float targetWallZ = aimTop ? -9.0f : 9.0f; // 壁の向こう側を狙うイメージ

	Vector3 target(targetWallX, 0.0f, targetWallZ);
	Vector3 dir = target - p;
	dir.Normalize();
	return dir;
}

// 安全なクリア方向
Vector3 CpuAI::GetSafeClearDir()
{
	Vector3 p = m_TargetPuck->GetTransform()->position;
	Vector3 enemyPos = m_EnemyPlayer->GetTransform()->position;

	// 敵が上にいれば下へ、下にいれば上へ
	float targetZ = (enemyPos.z > 0.0f) ? -0.8f : 0.8f;

	// 少し前方向成分も入れる
	Vector3 dir(-0.5f, 0.0f, targetZ);
	dir.Normalize();
	return dir;
}
