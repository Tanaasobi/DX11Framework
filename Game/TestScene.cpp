//==============================================================================
// TestScene.cpp - テスト用シーン実装
//==============================================================================

#include "TestScene.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Graphics/SkinnedMeshRenderer.h"
#include "Core/System/Input.h"
#include "Core/System/Time.h"
#include "Core/System/Logger.h"
#include "Core/Scene/SceneManager.h"

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
TestScene::TestScene()
	: Scene("TestScene")
{
	m_ToonSettings = {};
	m_OutlineSettings = {};
}

//==============================================================================
// デストラクタ
//==============================================================================
TestScene::~TestScene()
{
}

//==============================================================================
// 初期化
//==============================================================================
void TestScene::Init()
{
	Logger::Info("TestScene::Init()");

	//--------------------------------------------------------------------------
	// ライト設定
	//--------------------------------------------------------------------------
	LIGHT light;
	light.Direction = XMFLOAT4(0.5f, -0.5f, 0.7f, 0.0f);
	light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	light.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	Renderer::SetLight(light);

	//--------------------------------------------------------------------------
	// トゥーンシェーダー読み込み
	//--------------------------------------------------------------------------
	m_ToonShader = new ToonShader();
	m_ToonShader->LoadSkinned(
		L"Shader/ToonSkinnedVertexShader.hlsl",
		L"Shader/ToonPixelShader.hlsl"
	);
	m_ToonShader->LoadSkinnedOutline(
		L"Shader/ToonSkinnedOutlineVertexShader.hlsl",
		L"Shader/OutlinePixelShader.hlsl"
	);

	// トゥーン設定
	m_ToonSettings.Levels = 3;
	m_ToonSettings.Edge = 1.0f;
	m_ToonSettings.RimPower = 3.0f;
	m_ToonSettings.RimIntensity = 0.8f;
	m_ToonSettings.RimColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_ToonShader->SetToonSettings(m_ToonSettings);

	// アウトライン設定
	m_OutlineSettings.Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	m_OutlineSettings.Width = 0.7f;
	m_ToonShader->SetOutlineSettings(m_OutlineSettings);

	//--------------------------------------------------------------------------
	// モデル読み込み
	//--------------------------------------------------------------------------
	m_CharacterModel = new SkinnedModel();
	m_CharacterModel->Load("Asset/Model/character_base.fbx");

	//--------------------------------------------------------------------------
	// アニメーション読み込み
	//--------------------------------------------------------------------------
	m_IdleAnim = new AnimationClip();
	m_IdleAnim->Load("Asset/Anim/walk.fbx");
	//--------------------------------------------------------------------------
	// カメラ
	//--------------------------------------------------------------------------
	m_CameraObject = CreateGameObject("MainCamera");
	Camera* camera = m_CameraObject->AddComponent<Camera>();
	camera->SetAsMain();
	m_CameraObject->GetTransform()->position = Vector3(0.0f, 1.0f, -3.0f);

	//--------------------------------------------------------------------------
	// キャラクター
	//--------------------------------------------------------------------------
	m_Character = CreateGameObject("Character");

	// Animator
	Animator* animator = m_Character->AddComponent<Animator>();
	animator->SetSkeleton(m_CharacterModel->GetSkeleton());
	animator->Play(m_IdleAnim);

	// Transform調整
	m_Character->GetTransform()->position = Vector3(0.0f, 0.0f, 0.0f);
	m_Character->GetTransform()->scale = Vector3(0.01f, 0.01f, 0.01f);

	Logger::Info("TestScene initialized");
}

//==============================================================================
// 終了
//==============================================================================
void TestScene::Uninit()
{
	Logger::Info("TestScene::Uninit()");

	if (m_IdleAnim)
	{
		delete m_IdleAnim;
		m_IdleAnim = nullptr;
	}

	if (m_CharacterModel)
	{
		delete m_CharacterModel;
		m_CharacterModel = nullptr;
	}

	if (m_ToonShader)
	{
		delete m_ToonShader;
		m_ToonShader = nullptr;
	}
}

//==============================================================================
// 更新
//==============================================================================
void TestScene::Update(float deltaTime)
{
	// 基底クラスの更新
	Scene::Update(deltaTime);

	// キャラクター操作
	if (m_Character)
	{
		Transform* transform = m_Character->GetTransform();

		float speed = 2.0f * deltaTime;

		if (Input::GetKey(KeyCode::W)) transform->position.z += speed;
		if (Input::GetKey(KeyCode::S)) transform->position.z -= speed;
		if (Input::GetKey(KeyCode::A)) transform->position.x -= speed;
		if (Input::GetKey(KeyCode::D)) transform->position.x += speed;

		if (Input::GetKey(KeyCode::Q))
			transform->Rotate(Vector3(0.0f, -90.0f * deltaTime, 0.0f));
		if (Input::GetKey(KeyCode::E))
			transform->Rotate(Vector3(0.0f, 90.0f * deltaTime, 0.0f));
	}

	// カメラ操作
	if (m_CameraObject)
	{
		Transform* camTransform = m_CameraObject->GetTransform();
		float camSpeed = 3.0f * deltaTime;

		if (Input::GetKey(KeyCode::Up))    camTransform->position.z += camSpeed;
		if (Input::GetKey(KeyCode::Down))  camTransform->position.z -= camSpeed;
		if (Input::GetKey(KeyCode::Left))  camTransform->position.x -= camSpeed;
		if (Input::GetKey(KeyCode::Right)) camTransform->position.x += camSpeed;
	}

	// トゥーン設定の動的変更
	// 1-5キーで階調数変更
	if (Input::GetKeyDown(KeyCode::Num1)) { m_ToonSettings.Levels = 2; m_ToonShader->SetToonSettings(m_ToonSettings); Logger::Info("Toon Levels: 2"); }
	if (Input::GetKeyDown(KeyCode::Num2)) { m_ToonSettings.Levels = 3; m_ToonShader->SetToonSettings(m_ToonSettings); Logger::Info("Toon Levels: 3"); }
	if (Input::GetKeyDown(KeyCode::Num3)) { m_ToonSettings.Levels = 4; m_ToonShader->SetToonSettings(m_ToonSettings); Logger::Info("Toon Levels: 4"); }
	if (Input::GetKeyDown(KeyCode::Num4)) { m_ToonSettings.Levels = 5; m_ToonShader->SetToonSettings(m_ToonSettings); Logger::Info("Toon Levels: 5"); }
	if (Input::GetKeyDown(KeyCode::Num5)) { m_ToonSettings.Levels = 10; m_ToonShader->SetToonSettings(m_ToonSettings); Logger::Info("Toon Levels: 10"); }

	// Oキーでアウトラインのオン/オフ
	if (Input::GetKeyDown(KeyCode::O))
	{
		m_ToonShader->enableOutline = !m_ToonShader->enableOutline;
		Logger::InfoFormat("Outline: %s", m_ToonShader->enableOutline ? "ON" : "OFF");
	}

	// +/-キーでアウトライン太さ変更
	if (Input::GetKey(KeyCode::F1))
	{
		m_OutlineSettings.Width += 0.001f * deltaTime;
		m_ToonShader->SetOutlineSettings(m_OutlineSettings);
	}
	if (Input::GetKey(KeyCode::F2))
	{
		m_OutlineSettings.Width -= 0.001f * deltaTime;
		if (m_OutlineSettings.Width < 0.0f) m_OutlineSettings.Width = 0.0f;
		m_ToonShader->SetOutlineSettings(m_OutlineSettings);
	}

	// Rキーでシーンリロード
	if (Input::GetKeyDown(KeyCode::R))
	{
		SceneManager::ReloadCurrentScene();
	}
}

//==============================================================================
// 描画
//==============================================================================
void TestScene::Render()
{
	if (!m_Character || !m_ToonShader || !m_CharacterModel) return;

	Animator* animator = m_Character->GetComponent<Animator>();
	Transform* transform = m_Character->GetTransform();

	// ワールド行列設定
	Renderer::SetWorldMatrix(transform->GetWorldMatrix());

	// ボーン行列設定
	if (animator)
	{
		const auto& boneMatrices = animator->GetBoneMatrices();
		if (!boneMatrices.empty())
		{
			Renderer::SetBoneMatrices(boneMatrices.data(), static_cast<int>(boneMatrices.size()));
		}
	}

	//--------------------------------------------------------------------------
	// 1. アウトライン描画（背面カリング反転）
	//--------------------------------------------------------------------------
	if (m_ToonShader->enableOutline)
	{
		// 前面をカリング（背面を描画）
		Renderer::SetCullingMode(false);

		m_ToonShader->SetOutline();
		m_CharacterModel->Draw();

		// カリングを戻す
		Renderer::SetCullingMode(true);
	}

	//--------------------------------------------------------------------------
	// 2. メインモデル描画
	//--------------------------------------------------------------------------
	m_ToonShader->Set();
	m_CharacterModel->Draw();
}
