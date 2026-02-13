#pragma once

//==============================================================================
// GameScene.h - メインゲームシーン
//==============================================================================

#include "Core/Scene/Scene.h"
#include "Core/Graphics/Shader/IShader.h"
#include "Goal.h"
#include "GameState.h"

class Player;
class Puck;
class Goal;
class ScoreUI;
class CountdownUI;
class ParticleShader;
class ConfettiEmitter;
class ConfettiShader;
class RimLightSkinnedShader;
class ShadowMap;
class ShadowMapShader;
class FieldShadowShader;

class GameScene : public Scene
{
public:
	GameScene();
	virtual ~GameScene();

	void Init() override;
	void Uninit() override;
	void Update(float deltaTime) override;
	void Render() override;

private:
	// シェーダー類
	IShader* m_Shader = nullptr;
	IShader* m_SkinnedShader = nullptr;
	RimLightSkinnedShader* m_RimLightShader = nullptr;
	RimLightSkinnedShader* m_RimLightShaderCpu = nullptr;
	ParticleShader* m_ParticleShader = nullptr;
	ConfettiShader* m_ConfettiShader = nullptr;
	ConfettiEmitter* m_ConfettiEmitter = nullptr;

	// シャドウマップ関連
	ShadowMap* m_ShadowMap = nullptr;
	ShadowMapShader* m_ShadowMapShader = nullptr;
	FieldShadowShader* m_FieldShadowShader = nullptr;
	ID3D11Buffer* m_ShadowBuffer = nullptr;

	// ゲームオブジェクト
	Player* m_CachedPlayer = nullptr;
	Player* m_CachedCpu = nullptr;
	Puck* m_CachedPuck = nullptr;
	Goal* m_GoalLeft = nullptr;
	Goal* m_GoalRight = nullptr;

	// UI
	ScoreUI* m_ScoreUI = nullptr;
	CountdownUI* m_CountdownUI = nullptr;

	// ゲーム状態
	GameState m_GameState = GameState::Ready;
	float m_StateTimer = 0.0f;
	int m_CountdownValue = 3;
	Team m_LastScoredTeam = Team::Left;  // 最後に得点したチーム

	// 内部関数
	void CacheGameObjects();
	void TryKickPuck(); // プレイヤー用
	void CheckGoal();
	void SpawnGoalConfetti(Team scoringTeam);
	void ResetRound(Team scoredAgainstTeam);
	void ResetPositions();

	// 状態更新
	void UpdateReady(float deltaTime);
	void UpdateCountdown(float deltaTime);
	void UpdatePlaying(float deltaTime);
	void UpdateGoal(float deltaTime);
	void UpdateGameOver(float deltaTime);

	// シャドウマップ
	void RenderShadowMap();
	void UpdateShadowBuffer();

	// 状態遷移
	void ChangeState(GameState newState);
	void StartCountdown();
};
