#pragma once

//==============================================================================
// GameScene.h - メインゲームシーン
//==============================================================================

#include "Core/Scene/Scene.h"
#include "Core/Graphics/Shader/IShader.h"
#include "Game/Objects/Goal.h"
#include "Game/System/GameState.h"

class Player;
class CpuAI;
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
	IShader* m_Shader = nullptr;                          // 通常シェーダー
	IShader* m_SkinnedShader = nullptr;                   // スキンメッシュ用シェーダー
	RimLightSkinnedShader* m_RimLightShader = nullptr;    // 人間プレイヤー用リムライトシェーダー
	RimLightSkinnedShader* m_RimLightShaderCpu = nullptr; // CPUプレイヤー用リムライトシェーダー
	ParticleShader* m_ParticleShader = nullptr; 		  // パーティクルシェーダー
	ConfettiShader* m_ConfettiShader = nullptr; 	  	  // 紙吹雪シェーダー
	ConfettiEmitter* m_ConfettiEmitter = nullptr;         // 紙吹雪エミッター

	// シャドウマップ関連
	ShadowMap* m_ShadowMap = nullptr;                 // シャドウマップ
	ShadowMapShader* m_ShadowMapShader = nullptr;     // シャドウマップ生成用シェーダー
	FieldShadowShader* m_FieldShadowShader = nullptr; // フィールド影受け用シェーダー
	ID3D11Buffer* m_ShadowBuffer = nullptr;           // シャドウマップ用定数バッファ

	// ゲームオブジェクト
	Player* m_CachedPlayer = nullptr; // 人間プレイヤー
	Player* m_CachedCpu = nullptr;    // Cpuプレイヤー
	CpuAI* m_CachedCpuAI = nullptr;   // CpuAIコンポーネント
	Puck* m_CachedPuck = nullptr;     // パック
	Goal* m_GoalLeft = nullptr;       // 左ゴール
	Goal* m_GoalRight = nullptr;      // 右ゴール

	// UI
	ScoreUI* m_ScoreUI = nullptr;         // スコアUI
	CountdownUI* m_CountdownUI = nullptr; // カウントダウンUI

	// ゲーム状態
	GameState m_GameState = GameState::Ready;
	float m_StateTimer = 0.0f;
	int m_CountdownValue = 3;
	Team m_LastScoredTeam = Team::Left;  // 最後に得点したチーム

	// 内部関数
	void CacheGameObjects();
	void TryKickPuck();
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
