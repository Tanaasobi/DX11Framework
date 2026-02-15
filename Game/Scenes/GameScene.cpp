//==============================================================================
// GameScene.cpp - メインゲームシーン実装
//==============================================================================

#include "GameScene.h"
#include "Game/Objects/Player.h"
#include "Game/Components/CpuAI.h"
#include "Game/Components/CameraShake.h"
#include "Game/Objects/Puck.h"
#include "Game/Objects/Field.h"
#include "Game/Objects/Goal.h"
#include "Game/System/ScoreManager.h"
#include "Game/UI/ScoreUI.h"
#include "Game/UI/CountdownUI.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Graphics/Shader/Shader.h"
#include "Core/Graphics/Shader/SkinnedShader.h"
#include "Core/Graphics/Shader/RimLightSkinnedShader.h"
#include "Core/Graphics/Shader/ShadowMapShader.h"
#include "Core/Graphics/Shader/FieldShadowShader.h"
#include "Core/Graphics/Particle/ParticleShader.h"
#include "Core/Graphics/Particle/ConfettiEmitter.h"
#include "Core/Graphics/Particle/ConfettiShader.h"
#include "Core/Graphics/ShadowMap.h"
#include "Core/Graphics/Texture.h"
#include "Core/Audio/AudioComponent.h"
#include "Core/Object/Camera.h"
#include "Core/System/Input.h"
#include "Core/System/Logger.h"
#include "Core/Scene/SceneManager.h"
#include "Core/Physics/Collision.h"
#include "Core/Physics/Collider.h"
#include <cmath>


using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
GameScene::GameScene()
	: Scene("GameScene")
{
}

//==============================================================================
// デストラクタ
//==============================================================================
GameScene::~GameScene()
{
}

//==============================================================================
// 初期化
//==============================================================================
void GameScene::Init()
{
	Logger::Info("GameScene::Init()");

	ScoreManager::Instance().Reset();

	//--------------------------------------------------------------------------
	// シャドウマップ
	//--------------------------------------------------------------------------
	m_ShadowMap = new ShadowMap();
	m_ShadowMap->Init(2048, 2048);
	m_ShadowMap->SetLightDirection(1.0f, -1.5f, -1.0f);
	m_ShadowMap->SetLightTarget(0.0f, 0.0f, 0.0f);

	m_ShadowMapShader = new ShadowMapShader();
	m_ShadowMapShader->Load(L"Shader/ShadowMapVertexShader.hlsl", L"Shader/ShadowMapPixelShader.hlsl");

	m_FieldShadowShader = new FieldShadowShader();
	m_FieldShadowShader->Load(L"Shader/FieldVertexShader.hlsl", L"Shader/ShadowReceiverPixelShader.hlsl");

	// シャドウバッファ
	struct ShadowParams
	{
		XMMATRIX LightViewProj;
		float ShadowBias;
		float ShadowIntensity;
		float Padding[2];
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(ShadowParams);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_ShadowBuffer);

	//--------------------------------------------------------------------------
	// シェーダー
	//--------------------------------------------------------------------------
	Shader* normalShader = new Shader();
	normalShader->Load(L"Shader/VertexShader.hlsl", L"Shader/PixelShader.hlsl");
	m_Shader = normalShader;

	SkinnedShader* skinnedShader = new SkinnedShader();
	skinnedShader->Load(L"Shader/SkinnedVertexShader.hlsl", L"Shader/PixelShader.hlsl");
	m_SkinnedShader = skinnedShader;

	m_RimLightShader = new RimLightSkinnedShader();
	m_RimLightShader->Load(L"Shader/SkinnedVertexShader.hlsl", L"Shader/RimLightPixelShader.hlsl");
	m_RimLightShader->SetRimColor(0.2f, 0.6f, 1.0f);
	m_RimLightShader->SetRimPower(2.5f);
	m_RimLightShader->SetRimIntensity(1.5f);

	m_RimLightShaderCpu = new RimLightSkinnedShader();
	m_RimLightShaderCpu->Load(L"Shader/SkinnedVertexShader.hlsl", L"Shader/RimLightPixelShader.hlsl");
	m_RimLightShaderCpu->SetRimColor(1.0f, 0.3f, 0.2f);
	m_RimLightShaderCpu->SetRimPower(2.5f);
	m_RimLightShaderCpu->SetRimIntensity(1.5f);

	m_ParticleShader = new ParticleShader();
	m_ParticleShader->Load(L"Shader/ParticleVertexShader.hlsl", L"Shader/ParticlePixelShader.hlsl");

	m_ConfettiShader = new ConfettiShader();
	m_ConfettiShader->Load(L"Shader/ConfettiVertexShader.hlsl", L"Shader/ConfettiPixelShader.hlsl");

	//--------------------------------------------------------------------------
	// カメラ
	//--------------------------------------------------------------------------
	GameObject* cameraObj = CreateGameObject("MainCamera");
	Camera* camera = cameraObj->AddComponent<Camera>();
	camera->SetAsMain();
	camera->fov = 45.0f;
	cameraObj->GetTransform()->position = Vector3(0.0f, 35.0f, -20.0f);
	cameraObj->GetTransform()->SetEulerAngles(60.0f, 0.0f, 0.0f);
	// BGM再生
	auto bgmComp = cameraObj->AddComponent<AudioComponent>();  // カメラにAudioComponentを追加してBGMを再生
	bgmComp->Load("Asset/Audio/bgm.wav");
	bgmComp->SetVolume(0.1f);
	bgmComp->Play(true); // ループ再生
	// カメラシェイクコンポーネントも追加
	auto shakeComp = cameraObj->AddComponent<CameraShake>();
	
	//--------------------------------------------------------------------------
	// フィールド
	//--------------------------------------------------------------------------
	Field* field = new Field();
	field->Init(m_FieldShadowShader);
	AddGameObject(field);

	//--------------------------------------------------------------------------
	// ゴール
	//--------------------------------------------------------------------------
	Goal* goalLeft = new Goal(Team::Left);
	goalLeft->Init(m_Shader);
	auto goalComp = goalLeft->AddComponent<AudioComponent>();
	goalComp->Load("Asset/Audio/se_goal.wav");
	goalComp->SetVolume(0.7f);
	AddGameObject(goalLeft);

	Goal* goalRight = new Goal(Team::Right);
	goalRight->Init(m_Shader);
	goalComp = goalRight->AddComponent<AudioComponent>();
	goalComp->Load("Asset/Audio/se_goal.wav");
	goalComp->SetVolume(0.7f);
	AddGameObject(goalRight);

	//--------------------------------------------------------------------------
	// パック 
	//--------------------------------------------------------------------------
	Puck* puck = new Puck();
	puck->Init(m_Shader);
	auto hitComp = puck->AddComponent<AudioComponent>();
	hitComp->Load("Asset/Audio/se_hit.wav");
	hitComp->SetVolume(0.7f);
	AddGameObject(puck);

	//--------------------------------------------------------------------------
	// プレイヤー
	//--------------------------------------------------------------------------
	Player* player = new Player();
	player->Init(m_RimLightShader);
	AddGameObject(player);

	//--------------------------------------------------------------------------
	// CPU
	//--------------------------------------------------------------------------
	Player* cpu = new Player();
	cpu->SetName("CPU");
	cpu->Init(m_RimLightShaderCpu);
	cpu->GetTransform()->position = Vector3(8.0f, 0.0f, 0.0f);

	// AIコンポーネント追加
	CpuAI* ai = cpu->AddComponent<CpuAI>();
	ai->Init(cpu, puck);
	m_CachedCpuAI = ai;

	AddGameObject(cpu);

	//--------------------------------------------------------------------------
	// 紙吹雪
	//--------------------------------------------------------------------------
	GameObject* confettiObj = CreateGameObject("Confetti");
	m_ConfettiEmitter = confettiObj->AddComponent<ConfettiEmitter>();
	m_ConfettiEmitter->Init(10000);
	m_ConfettiEmitter->SetShader(m_ConfettiShader);
	m_ConfettiEmitter->SetTexture(Texture::Load("Asset/Texture/white.png"));

	// パラメータ調整
	m_ConfettiEmitter->initialSpeedMin = 5.0f;
	m_ConfettiEmitter->initialSpeedMax = 40.0f;
	m_ConfettiEmitter->spreadX = 20.0f;
	m_ConfettiEmitter->spreadY = 5.0f;
	m_ConfettiEmitter->spreadZ = 20.0f;
	m_ConfettiEmitter->gravity = 9.0f;

	//--------------------------------------------------------------------------
	// UI
	//--------------------------------------------------------------------------
	GameObject* scoreUIObj = CreateGameObject("ScoreUI");
	m_ScoreUI = scoreUIObj->AddComponent<ScoreUI>();
	m_ScoreUI->Init();
	m_ScoreUI->leftScoreX = 100.0f;
	m_ScoreUI->rightScoreX = SCREEN_WIDTH - 100.0f;
	m_ScoreUI->scoreY = 50.0f;

	GameObject* countdownUIObj = CreateGameObject("CountdownUI");
	m_CountdownUI = countdownUIObj->AddComponent<CountdownUI>();
	m_CountdownUI->Init();

	//--------------------------------------------------------------------------
	// キャッシュ
	//--------------------------------------------------------------------------
	CacheGameObjects();

	//--------------------------------------------------------------------------
	// 初期状態
	//--------------------------------------------------------------------------
	ResetPositions();
	ChangeState(GameState::Ready);

	Logger::Info("GameScene initialized");
}

//==============================================================================
// 終了
//==============================================================================
void GameScene::Uninit()
{
	Logger::Info("GameScene::Uninit()");

	m_CachedPlayer = nullptr;
	m_CachedCpu = nullptr;
	m_CachedCpuAI = nullptr;
	m_CachedPuck = nullptr;
	m_GoalLeft = nullptr;
	m_GoalRight = nullptr;
	m_ScoreUI = nullptr;
	m_CountdownUI = nullptr;
	m_ConfettiEmitter = nullptr;

	SAFE_RELEASE(m_ShadowBuffer);

	if (m_ShadowMap) { delete m_ShadowMap; m_ShadowMap = nullptr; }
	if (m_ShadowMapShader) { delete m_ShadowMapShader; m_ShadowMapShader = nullptr; }
	if (m_FieldShadowShader) { delete m_FieldShadowShader; m_FieldShadowShader = nullptr; }
	if (m_Shader) { delete m_Shader; m_Shader = nullptr; }
	if (m_SkinnedShader) { delete m_SkinnedShader; m_SkinnedShader = nullptr; }
	if (m_RimLightShader) { delete m_RimLightShader; m_RimLightShader = nullptr; }
	if (m_RimLightShaderCpu) { delete m_RimLightShaderCpu; m_RimLightShaderCpu = nullptr; }
	if (m_ParticleShader) { delete m_ParticleShader; m_ParticleShader = nullptr; }
	if (m_ConfettiShader) { delete m_ConfettiShader; m_ConfettiShader = nullptr; }
}

//==============================================================================
// キャッシュ取得
//==============================================================================
void GameScene::CacheGameObjects()
{
	m_CachedPlayer = FindGameObjectOfType<Player>();
	m_CachedPuck = FindGameObjectOfType<Puck>();
	m_GoalLeft = dynamic_cast<Goal*>(FindGameObject("GoalLeft"));
	m_GoalRight = dynamic_cast<Goal*>(FindGameObject("GoalRight"));

	for (auto& obj : m_GameObjects)
	{
		Player* p = dynamic_cast<Player*>(obj.get());
		if (p && p->GetName() == "CPU")
		{
			m_CachedCpu = p;
			break;
		}
	}
}

//==============================================================================
// 更新
//==============================================================================
void GameScene::Update(float deltaTime)
{
	Scene::Update(deltaTime);

	// 状態に応じた更新
	switch (m_GameState)
	{
	case GameState::Ready:
		UpdateReady(deltaTime);
		break;
	case GameState::Countdown:
		UpdateCountdown(deltaTime);
		break;
	case GameState::Playing:
		UpdatePlaying(deltaTime);
		break;
	case GameState::Goal:
		UpdateGoal(deltaTime);
		break;
	case GameState::GameOver:
		UpdateGameOver(deltaTime);
		break;
	}

	// リロード（いつでも可能）
	if (Input::GetKeyDown(KeyCode::R))
	{
		SceneManager::ReloadCurrentScene();
	}
}

//==============================================================================
// Ready状態
//==============================================================================
void GameScene::UpdateReady(float deltaTime)
{
	m_StateTimer += deltaTime;

	// 1秒待ってカウントダウン開始
	if (m_StateTimer >= 1.0f)
	{
		StartCountdown();
	}
}

//==============================================================================
// Countdown状態
//==============================================================================
void GameScene::UpdateCountdown(float deltaTime)
{
	m_StateTimer += deltaTime;

	// 1秒ごとにカウントダウン
	if (m_StateTimer >= 1.0f)
	{
		m_StateTimer = 0.0f;
		m_CountdownValue--;

		if (m_CountdownValue > 0)
		{
			m_CountdownUI->SetCount(m_CountdownValue);
		}
		else if (m_CountdownValue == 0)
		{
			// GO!
			m_CountdownUI->SetCount(0);
		}
		else
		{
			// カウントダウン終了、プレイ開始
			m_CountdownUI->Hide();
			ChangeState(GameState::Playing);
		}
	}
}

//==============================================================================
// Playing状態
//==============================================================================
void GameScene::UpdatePlaying(float deltaTime)
{
	// プレイヤーの蹴り
	if (Input::GetKeyDown(KeyCode::MouseLeft) || Input::GetKeyDown(KeyCode::Space) ||
		Input::GetGamepadRightTrigger(0) > 0.3f)
	{
		TryKickPuck();
	}


	// ゴール判定
	CheckGoal();
}

//==============================================================================
// Goal状態（演出中）
//==============================================================================
void GameScene::UpdateGoal(float deltaTime)
{
	m_StateTimer += deltaTime;

	// 2秒待って次のラウンド
	if (m_StateTimer >= 2.0f)
	{
		if (ScoreManager::Instance().IsGameOver())
		{
			ChangeState(GameState::GameOver);
		}
		else
		{
			ResetRound(m_LastScoredTeam);
			StartCountdown();
		}
	}
}

//==============================================================================
// GameOver状態
//==============================================================================
void GameScene::UpdateGameOver(float deltaTime)
{
	// スペースキーまたはクリックでリザルトへ
	if (Input::GetKeyDown(KeyCode::Space) || Input::GetKeyDown(KeyCode::MouseLeft) ||
		Input::GetGamepadButtonDown(0, GamepadButton::A))
	{
		SceneManager::LoadScene("ResultScene");
	}
}

//==============================================================================
// 状態遷移
//==============================================================================
void GameScene::ChangeState(GameState newState)
{
	m_GameState = newState;
	m_StateTimer = 0.0f;

	// 状態に応じて入力を制御
	bool enableInput = (newState == GameState::Playing);

	if (m_CachedPlayer)
	{
		m_CachedPlayer->SetInputEnabled(enableInput);
	}

	if (m_CachedCpu)
	{
		m_CachedCpu->SetInputEnabled(enableInput);
		m_CachedCpu->SetMoveInput(0.0f, 0.0f);  // 移動入力もクリア
	}

	Logger::InfoFormat("GameState changed to: %d", static_cast<int>(newState));
}

//==============================================================================
// カウントダウン開始
//==============================================================================
void GameScene::StartCountdown()
{
	m_CountdownValue = 3;
	m_CountdownUI->SetCount(m_CountdownValue);
	ChangeState(GameState::Countdown);
}

//==============================================================================
// 位置リセット
//==============================================================================
void GameScene::ResetPositions()
{
	// プレイヤー位置
	if (m_CachedPlayer)
	{
		m_CachedPlayer->GetTransform()->position = Vector3(-8.0f, 0.0f, 0.0f);
		m_CachedPlayer->GetTransform()->SetEulerAngles(0.0f, -90.0f, 0.0f);
	}

	// CPU位置
	if (m_CachedCpu)
	{
		m_CachedCpu->GetTransform()->position = Vector3(8.0f, 0.0f, 0.0f);
		m_CachedCpu->GetTransform()->SetEulerAngles(0.0f, 90.0f, 0.0f);
	}

	// パック位置（中央）
	if (m_CachedPuck)
	{
		m_CachedPuck->GetTransform()->position = Vector3(0.0f, 0.5f, 0.0f);
		m_CachedPuck->SetVelocity(0.0f, 0.0f);
	}
}

//==============================================================================
// ラウンドリセット（ゴール後）
//==============================================================================
void GameScene::ResetRound(Team scoredAgainstTeam)
{
	// プレイヤー位置リセット
	if (m_CachedPlayer)
	{
		m_CachedPlayer->GetTransform()->position = Vector3(-8.0f, 0.0f, 0.0f);
		m_CachedPlayer->GetTransform()->SetEulerAngles(0.0f, -90.0f, 0.0f);
	}

	if (m_CachedCpu)
	{
		m_CachedCpu->GetTransform()->position = Vector3(8.0f, 0.0f, 0.0f);
		m_CachedCpu->GetTransform()->SetEulerAngles(0.0f, 90.0f, 0.0f);
	}

	// パックは得点された側に配置
	if (m_CachedPuck)
	{
		float puckX = (scoredAgainstTeam == Team::Left) ? -5.0f : 5.0f;
		m_CachedPuck->GetTransform()->position = Vector3(puckX, 0.5f, 0.0f);
		m_CachedPuck->SetVelocity(0.0f, 0.0f);
	}
}

//==============================================================================
// ゴール判定
//==============================================================================
void GameScene::CheckGoal()
{
	if (!m_CachedPuck || !m_GoalLeft || !m_GoalRight) return;

	CircleCollider* puckCol = m_CachedPuck->GetCollider();
	if (!puckCol) return;

	// 左ゴール判定（右チームが得点 = 左チームがやられた）
	BoxCollider* leftGoalCol = m_GoalLeft->GetCollider();
	if (leftGoalCol && Collision::CircleVsBox(puckCol, leftGoalCol))
	{
		ScoreManager::Instance().AddScore(Team::Right);
		SpawnGoalConfetti(Team::Right);
		m_LastScoredTeam = Team::Left;

		// パックを即座に停止・非表示位置に移動
		m_CachedPuck->SetVelocity(0.0f, 0.0f);
		m_CachedPuck->GetTransform()->position = Vector3(0.0f, -10.0f, 0.0f);  // 画面外

		// ゴール音再生
		auto goalAudio = m_GoalLeft->GetComponent<AudioComponent>();
		if (goalAudio)
		{
			goalAudio->Play();
		}

		ChangeState(GameState::Goal);
		return;
	}

	// 右ゴール判定（左チームが得点 = 右チームがやられた）
	BoxCollider* rightGoalCol = m_GoalRight->GetCollider();
	if (rightGoalCol && Collision::CircleVsBox(puckCol, rightGoalCol))
	{
		ScoreManager::Instance().AddScore(Team::Left);
		SpawnGoalConfetti(Team::Left);
		m_LastScoredTeam = Team::Right;

		// パックを即座に停止・非表示位置に移動
		m_CachedPuck->SetVelocity(0.0f, 0.0f);
		m_CachedPuck->GetTransform()->position = Vector3(0.0f, -10.0f, 0.0f);  // 画面外

		// ゴール音再生
		auto goalAudio = m_GoalRight->GetComponent<AudioComponent>();
		if (goalAudio)
		{
			goalAudio->Play();
		}

		ChangeState(GameState::Goal);
		return;
	}
}

//==============================================================================
// ゴール時の紙吹雪
//==============================================================================
void GameScene::SpawnGoalConfetti(Team scoringTeam)
{
	if (!m_ConfettiEmitter) return;

	Vector3 spawnPos;
	Vector3 direction;

	if (scoringTeam == Team::Left)
	{
		spawnPos = Vector3(FieldBounds::RIGHT - 2.0f, 3.0f, 0.0f);
		direction = Vector3(-1.0f, 0.0f, 0.0f);
	}
	else
	{
		spawnPos = Vector3(FieldBounds::LEFT + 2.0f, 3.0f, 0.0f);
		direction = Vector3(1.0f, 0.0f, 0.0f);
	}

	m_ConfettiEmitter->Burst(4000, spawnPos, direction);
}

//==============================================================================
// 蹴る処理（プレイヤー用）
//==============================================================================
void GameScene::TryKickPuck()
{
	if (m_CachedPlayer && m_CachedPuck)
	{
		// プレイヤーの入力に基づいてキック
		m_CachedPlayer->Kick(m_CachedPuck);
	}
}

//==============================================================================
// 描画
//==============================================================================
void GameScene::Render()
{
	UpdateShadowBuffer();
	RenderShadowMap();

	Camera* camera = Camera::GetMain();
	if (camera)
	{
		camera->SetMatrices();
	}

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	if (m_ShadowMap)
	{
		ID3D11ShaderResourceView* shadowSRV = m_ShadowMap->GetShadowMapSRV();
		context->PSSetShaderResources(1, 1, &shadowSRV);
	}

	if (m_ShadowBuffer)
	{
		context->PSSetConstantBuffers(8, 1, &m_ShadowBuffer);
	}

	Scene::Render();

	// シャドウマップSRVのバインド解除
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->PSSetShaderResources(1, 1, &nullSRV);

	// デバッグUIの描画
	if (m_CachedCpuAI)
	{
		m_CachedCpuAI->DrawDebugGUI();
	}
}

//==============================================================================
// シャドウマップ描画
//==============================================================================
void GameScene::RenderShadowMap()
{
	if (!m_ShadowMap || !m_ShadowMapShader) return;

	m_ShadowMap->Begin();
	m_ShadowMapShader->Set();

	if (m_CachedPlayer) m_CachedPlayer->Render();
	if (m_CachedCpu) m_CachedCpu->Render();

	m_ShadowMap->End();
}

//==============================================================================
// シャドウバッファ更新
//==============================================================================
void GameScene::UpdateShadowBuffer()
{
	if (!m_ShadowMap || !m_ShadowBuffer) return;

	struct ShadowParams
	{
		XMMATRIX LightViewProj;
		float ShadowBias;
		float ShadowIntensity;
		float Padding[2];
	};

	ShadowParams params;
	params.LightViewProj = XMMatrixTranspose(
		m_ShadowMap->GetLightViewMatrix() * m_ShadowMap->GetLightProjectionMatrix()
	);
	params.ShadowBias = 0.002f;
	params.ShadowIntensity = 0.35f;

	Renderer::GetDeviceContext()->UpdateSubresource(m_ShadowBuffer, 0, nullptr, &params, 0, 0);
}
