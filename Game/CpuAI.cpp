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
#include "Core/Scene/Scene.h"
#include "Core/Graphics/UI/TextRenderer.h"
#include "Core/System/main.h"
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
CpuAI::CpuAI()
	: m_Rng(std::random_device{}())
{
}

//==============================================================================
// デストラクタ
//==============================================================================
CpuAI::~CpuAI()
{
	if (m_DebugFont)
	{
		m_DebugFont->Uninit();
		delete m_DebugFont;
		m_DebugFont = nullptr;
	}
}

//==============================================================================
// 初期化
//==============================================================================
void CpuAI::Init(Player* ownerPlayer, Puck* targetPuck)
{
	m_Owner = ownerPlayer;
	m_TargetPuck = targetPuck;

	// デバッグ用フォント初期化
	m_DebugFont = new Font();
	m_DebugFont->Init(L"Yu Gothic", 24.0f);

	// 相手プレイヤーを探す
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
			Logger::Warning("CpuAI: Enemy player not found (found self).");
		}
	}

	if (m_Owner)
	{
		m_Owner->EnableExternalInput(true);
	}
}

//==============================================================================
// 更新
//==============================================================================
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

	// キック判定だけは毎フレーム行う
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;
	Vector3 myPos = m_Owner->GetTransform()->position;

	// キック可能範囲内に入ったら即キック
	Vector3 diff = myPos - puckPos;
	float distSq = diff.LengthSquared(); // x*x + y*y + z*z

	if (distSq < 3.0f * 3.0f && m_Owner->CanKick())
	{
		// 自陣深くならクリア、敵陣寄りならシュート
		if (puckPos.x > 0.0f)
		{
			m_State = CpuState::Clear;
			m_DebugStateStr = "Action: CLEAR (Instant)";
		}
		else
		{
			m_State = CpuState::Shoot;
			m_DebugStateStr = "Action: SHOOT (Instant)";
		}
		m_ActionTimer = 0.2f; // アクション予備動作時間
		return;
	}

	// 移動方針（攻守）の決定は、これまで通り一定間隔で行う（ジッタリング防止）
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

	// --- 到達時間計算 ---
	float myReachTime = CalculateReachTime(myPos, m_Owner->moveSpeed, puckPos);
	float enemyReachTime = 100.0f;

	if (m_EnemyPlayer)
	{
		enemyReachTime = CalculateReachTime(m_EnemyPlayer->GetTransform()->position,
			m_EnemyPlayer->moveSpeed, puckPos);
	}

	// --- ステート決定ロジック ---
	bool isPuckComing = (puckVelX > 0.5f); // パックが自陣（右）に向かっている
	bool isPuckInMySide = (puckPos.x > 0.0f); // パックが自陣にある

	// デバッグ情報の更新
	std::ostringstream ss;
	ss << "MyReach: " << std::fixed << std::setprecision(2) << myReachTime
		<< " / EnemyReach: " << enemyReachTime;
	m_DebugInfoStr = ss.str();

	if (isPuckInMySide)
	{
		// 自陣にある場合
		// 多少遅くても、自陣なら無理して取りに行く
		if (myReachTime < enemyReachTime * 1.2f)
		{
			m_State = CpuState::Attack;
			m_DebugStateStr = "State: ATTACK (My Side)";
		}
		else
		{
			m_State = CpuState::Defend;
			m_DebugStateStr = "State: DEFEND (Too far)";
		}
	}
	else
	{
		// 敵陣にある場合
		// 対等なら攻める
		if (myReachTime < enemyReachTime * 1.0f)
		{
			m_State = CpuState::Attack;
			m_DebugStateStr = "State: ATTACK (Enemy Side)";
		}
		else if (isPuckComing)
		{
			m_State = CpuState::Defend;
			m_DebugStateStr = "State: DEFEND (Counter)";
		}
		else
		{
			// 敵陣で止まっている、または敵がキープしている
			// 距離が近ければプレッシャーをかける
			if (myReachTime < 3.0f)
			{
				m_State = CpuState::Attack;
				m_DebugStateStr = "State: ATTACK (Pressure)";
			}
			else
			{
				m_State = CpuState::Wait;
				m_DebugStateStr = "State: WAIT";
			}
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
	Vector3 myPos = m_Owner->GetTransform()->position;

	// パックが自分より右（自陣ゴール寄り）にある場合
	if (puckPos.x > myPos.x)
	{
		// ステップシミュレーションを使って「最短で触れる場所」へ移動
		// バウンド前も考慮される
		Vector3 target = CalculateInterceptionPos();
		MoveTo(target);
		return;
	}

	// 通常（前方にある場合）は直近の未来位置を追う
	float vx, vz;
	m_TargetPuck->GetVelocity(vx, vz);
	Vector3 target = puckPos + Vector3(vx, 0, vz) * 0.2f;

	MoveTo(target);
}

//------------------------------------------------------------------------------
// 守り: ゴールとパックを結ぶ直線を塞ぐ
//------------------------------------------------------------------------------
void CpuAI::UpdateDefend()
{
	Vector3 puckPos = m_TargetPuck->GetTransform()->position;

	// 自陣ゴールの位置（FieldBounds::RIGHT の中心）
	// ゴールラインより少し手前を守備ラインとする
	const float GOAL_X = FieldBounds::RIGHT;

	// 守備ラインのX座標（ゴールの少し前で待つ）
	// あまりゴールに張り付くと横を抜かれるので、適度な距離(DEFEND_LINE_X)を保つ
	// ただし、パックが守備ラインより後ろに来た場合は、パックとゴールの間に潜り込む
	float targetX = DEFEND_LINE_X;

	// パックが守備ラインを超えて自陣ゴール側に来ている場合
	if (puckPos.x > DEFEND_LINE_X)
	{
		// パックとゴールの間（パックの少し後ろ）に入る
		targetX = puckPos.x + 2.0f;
		// ゴールより後ろには行かない
		targetX = std::min(targetX, GOAL_X - 1.0f);
	}

	// --- シュートコースの遮断計算 ---
	// 直線方程式: ゴール(G)とパック(P)を結ぶ線分上で、X座標が targetX となる点の Z を求める
	// 比率 t = (TargetX - G.x) / (P.x - G.x)
	// TargetZ = G.z + (P.z - G.z) * t

	// ゴールのZは0なので、式はシンプルになる: TargetZ = P.z * t

	float denominator = puckPos.x - GOAL_X;
	float targetZ = 0.0f;

	// ゼロ除算防止（パックがゴールラインと重なることはほぼないが念のため）
	if (std::abs(denominator) > 0.001f)
	{
		float t = (targetX - GOAL_X) / denominator;
		targetZ = puckPos.z * t;
	}
	else
	{
		targetZ = puckPos.z;
	}

	// 目標地点
	Vector3 targetPos(targetX, 0.0f, targetZ);

	// 移動
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
			m_DebugInfoStr = "Shot: BOUNCE";
		}
		else
		{
			shootDir = GetDirectShotDir();
			m_DebugInfoStr = "Shot: DIRECT";
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
			m_DebugInfoStr = "Clear: SAFE";
		}
		else
		{
			// とりあえず前（敵ゴール方向）へ強く蹴る
			clearDir = Vector3(-1.0f, 0.0f, (dist(m_Rng) % 2 == 0) ? 0.5f : -0.5f);
			clearDir.Normalize();
			m_DebugInfoStr = "Clear: RAND";
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

	Vector3 diff = targetPos - myPos;
	float dist = diff.Length();

	// 停止閾値（ジッタリング対策）
	const float STOP_THRESHOLD = 0.2f;

	if (dist > STOP_THRESHOLD)
	{
		// 目的地に近づいたら減速する処理 (Arrive挙動)
		const float SLOW_DOWN_DIST = 1.5f; // 減速を開始する距離
		float magnitude = 1.0f;

		if (dist < SLOW_DOWN_DIST)
		{
			// 距離に応じて入力を弱める
			magnitude = dist / SLOW_DOWN_DIST;
			magnitude = std::max(magnitude, 0.2f);
		}

		m_Owner->SetMoveInput((diff.x / dist) * magnitude, (diff.z / dist) * magnitude);
	}
	else
	{
		m_Owner->SetMoveInput(0.0f, 0.0f);
	}
}

bool CpuAI::TryKick(const Vector3& dir)
{
	if (m_Owner && m_TargetPuck)
	{
		// 方向を指定してキック
		return m_Owner->Kick(m_TargetPuck, dir);
	}
	return false;
}

// パックの先読み（壁反射考慮）
Vector3 CpuAI::PredictPuckPosOnLine(float targetX)
{
	Vector3 p = m_TargetPuck->GetTransform()->position;
	float vx, vz;
	m_TargetPuck->GetVelocity(vx, vz);

	if (std::abs(vx) < 0.01f) return p;
	if ((targetX - p.x) * vx < 0) return p;

	float t = (targetX - p.x) / vx;
	float predictedZ = p.z + vz * t;

	// FieldBoundsを使用
	while (predictedZ < FieldBounds::TOP || predictedZ > FieldBounds::BOTTOM)
	{
		if (predictedZ < FieldBounds::TOP)
		{
			predictedZ = FieldBounds::TOP + (FieldBounds::TOP - predictedZ);
		}
		else if (predictedZ > FieldBounds::BOTTOM)
		{
			predictedZ = FieldBounds::BOTTOM - (predictedZ - FieldBounds::BOTTOM);
		}
	}

	return Vector3(targetX, 0.0f, predictedZ);
}

// 直接シュート方向（相手ゴール中心）
Vector3 CpuAI::GetDirectShotDir()
{
	Vector3 p = m_TargetPuck->GetTransform()->position;

	// FieldBoundsを使用
	Vector3 target(FieldBounds::LEFT, 0.0f, 0.0f);

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

	// ゴールの鏡像を狙う
	float mirrorZ = aimTop ? (FieldBounds::TOP * 2.0f) : (FieldBounds::BOTTOM * 2.0f);

	Vector3 target(FieldBounds::LEFT, 0.0f, mirrorZ);

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

// 最短迎撃ポイントの計算（シミュレーション）
Vector3 CpuAI::CalculateInterceptionPos()
{
	Vector3 p = m_TargetPuck->GetTransform()->position;
	float vx, vz;
	m_TargetPuck->GetVelocity(vx, vz);
	Vector3 v(vx, 0.0f, vz);

	Vector3 myPos = m_Owner->GetTransform()->position;
	float mySpeed = m_Owner->moveSpeed;
	if (mySpeed <= 0.0f) return p;

	const float dt = 0.05f;
	const float maxTime = 2.0f;
	const float GOAL_HALF_WIDTH = 4.0f; // ゴール幅（適宜調整）

	for (float t = 0.0f; t < maxTime; t += dt)
	{
		p.x += v.x * dt;
		p.z += v.z * dt;

		// FieldBoundsを使用
		if (p.z < FieldBounds::TOP)
		{
			p.z = FieldBounds::TOP + (FieldBounds::TOP - p.z);
			v.z *= -1.0f;
		}
		else if (p.z > FieldBounds::BOTTOM)
		{
			p.z = FieldBounds::BOTTOM - (p.z - FieldBounds::BOTTOM);
			v.z *= -1.0f;
		}

		// 自陣ゴール（RIGHT）に入りそうなら手前で止める
		if (p.x > FieldBounds::RIGHT && std::abs(p.z) < GOAL_HALF_WIDTH)
		{
			p.x = FieldBounds::RIGHT - 0.5f;
			return p;
		}

		Vector3 diff = p - myPos;
		float dist = diff.Length();
		float reachTime = dist / mySpeed;

		if (reachTime <= t)
		{
			return p;
		}
	}

	return p;
}

// デバッグ描画
void CpuAI::DrawDebugGUI()
{
	if (!m_DebugFont) return;

	// 画面右下に表示
	float startX = SCREEN_WIDTH - 20.0f;
	float startY = SCREEN_HEIGHT - 80.0f;

	// ステート表示
	std::wstring stateW = std::wstring(m_DebugStateStr.begin(), m_DebugStateStr.end());
	TextRenderer::Draw(m_DebugFont, stateW, startX, startY, 1.0f, 0.2f, 0.2f, 1.0f, TextAlign::Right);

	// 数値情報表示
	std::wstring infoW = std::wstring(m_DebugInfoStr.begin(), m_DebugInfoStr.end());
	TextRenderer::Draw(m_DebugFont, infoW, startX, startY + 30.0f, 0.8f, 0.8f, 0.8f, 1.0f, TextAlign::Right);
}
