#pragma once

//==============================================================================
// CpuAI.h - CPUの思考ロジックコンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/Math/Vector3.h"
#include <random>

class Player;
class Puck;

// AIの状態
enum class CpuState
{
	Idle,       // 待機
	Defend,     // 守備
	Attack,     // 攻撃
	Shoot       // シュート
};

class CpuAI : public Component
{
public:
	CpuAI();
	virtual ~CpuAI();

	void Init(Player* ownerPlayer, Puck* targetPuck);
	void Update(float deltaTime) override;

	// 外部から設定を変更する場合
	void SetActive(bool active) { m_IsActive = active; }

private:
	// 参照
	Player* m_Owner = nullptr;
	Puck* m_TargetPuck = nullptr;

	// AIパラメータ
	bool m_IsActive = true;
	CpuState m_State = CpuState::Idle;

	// 乱数生成
	std::mt19937 m_Rng;
	std::uniform_real_distribution<float> m_KickIntervalRand;

	// タイマー類
	float m_KickTimer = 0.0f;
	float m_KickInterval = 0.7f;

	// 定数（調整が必要なら外部化する）
	const float DEFEND_LINE_X = 4.0f;
	const float HOME_POS_X = 6.0f;

	// 内部ロジック
	void UpdateState();
	void ExecuteAction(float deltaTime);
	void MoveToTarget(const Vector3& targetPos);
	void GetShootDirection(float& outDirX, float& outDirZ);
};
