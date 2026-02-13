#pragma once

//==============================================================================
// CpuAI.h - 強化版CPUロジック
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/Math/Vector3.h"
#include "Core/Graphics/UI/Font.h"
#include <string>
#include <random>

class Player;
class Puck;

// AIの状態
enum class CpuState
{
	Wait,       // 待機（自陣で様子見）
	Defend,     // 守備（コースを塞ぐ・予測位置へ移動）
	Attack,     // 攻撃（パックを奪いにいく）
	Shoot,      // シュート動作中
	Clear       // クリア動作中（守備からの蹴り出し）
};

class CpuAI : public Component
{
public:
	CpuAI();
	virtual ~CpuAI();

	void Init(Player* ownerPlayer, Puck* targetPuck);
	void Update(float deltaTime) override;

	// パラメータ設定（調整用）
	void SetActive(bool active) { m_IsActive = active; }

	// デバッグ描画メソッド
	void DrawDebugGUI();

private:
	// 参照
	Player* m_Owner = nullptr;
	Puck* m_TargetPuck = nullptr;
	Player* m_EnemyPlayer = nullptr; // 相手プレイヤー（位置把握用）

	// 状態
	bool m_IsActive = true;
	CpuState m_State = CpuState::Wait;

	// 乱数
	std::mt19937 m_Rng;

	// 行動制御タイマー
	float m_ActionTimer = 0.0f;
	float m_ThinkInterval = 0.1f; // 思考更新間隔（毎フレーム判断するとブレるので）
	float m_ThinkTimer = 0.0f;

	// パラメータ定数
	const float DEFEND_LINE_X = 5.0f;     // 守備時の基本ライン（自陣ゴール前）
	const float WAIT_POS_X = 3.0f;        // 待機時のライン
	const float ATTACK_THRESHOLD_Z = 0.5f;// 攻撃時にどれくらい軸が合ったら突っ込むか

	// 確率パラメータ (%)
	const int PROB_WALL_SHOT = 40;        // 壁打ちシュートの確率
	const int PROB_CLEAR_SAFE = 70;       // 安全な方向へクリアする確率

	// フィールド定数（シミュレーション用）
	const float SIM_FIELD_TOP = -8.0f;
	const float SIM_FIELD_BOTTOM = 8.0f;
	const float SIM_FIELD_RIGHT = 11.0f; // 右ゴールライン（自陣）
	const float SIM_GOAL_HALF_WIDTH = 4.0f; // ゴールのZ幅の半分

	// デバッグ用
	Font* m_DebugFont = nullptr;
	std::string m_DebugStateStr; // 状態名文字列
	std::string m_DebugInfoStr;  // 数値情報文字列

	//--------------------------------------------------------------------------
	// 内部ロジック
	//--------------------------------------------------------------------------

	// 判断
	void UpdateDecision(float deltaTime);
	void DecideState();
	float CalculateReachTime(const Vector3& fromPos, float moveSpeed, const Vector3& targetPos);

	// 行動実行
	void ExecuteAction(float deltaTime);

	// 各状態の挙動
	void UpdateAttack();
	void UpdateDefend();
	void UpdateWait();
	void UpdateShoot(float deltaTime);
	void UpdateClear(float deltaTime);

	// アクションヘルパー
	void MoveTo(const Vector3& targetPos);
	bool TryKick(const Vector3& dir);

	// 計算ヘルパー
	Vector3 PredictPuckPosOnLine(float targetX); // パックがXラインに到達する位置を予測（壁反射考慮）
	Vector3 GetDirectShotDir();                  // ゴールへの直接シュート方向
	Vector3 GetBounceShotDir();                  // 壁反射シュート方向
	Vector3 GetSafeClearDir();                   // 敵がいない方向へのクリア方向

	// 最短迎撃ポイントの計算（ステップシミュレーション）
	Vector3 CalculateInterceptionPos();
};
