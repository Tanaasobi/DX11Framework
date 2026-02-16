//==============================================================================
// Puck.cpp - パック実装
//==============================================================================

#include "Puck.h"
#include "Field.h"
#include "Core/Graphics/ModelRenderer.h"
#include "Core/Graphics/Model.h"        
#include "Core/Physics/Collider.h"
#include "Core/Physics/Collision.h"
#include "Core/System/Logger.h"
#include "Core/Math/Vector3.h"
#include "Game/Components/TrailRenderer.h"
#include "Core/Graphics/Particle/ParticleEmitter.h"
#include "Core/Graphics/Particle/ParticleShader.h"

//==============================================================================
// コンストラクタ
//==============================================================================
Puck::Puck()
	: GameObject("Puck")
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Puck::~Puck()
{
	// モデルのメモリ解放
	if (m_Model)
	{
		delete m_Model;
		m_Model = nullptr;
	}

	// 火花エフェクトのシェーダー解放
	if (m_SparkShader)
	{
		delete m_SparkShader;
		m_SparkShader = nullptr;
	}
}

//==============================================================================
// 初期化
//==============================================================================
void Puck::Init(IShader* shader)
{
	//--------------------------------------------------------------------------
	// モデル読み込み & レンダラー設定
	//--------------------------------------------------------------------------
	m_Model = new Model();
	// モデルのパスを指定 (Assetフォルダ内のパスを確認してください)
	if (!m_Model->Load("Asset/Model/puck.fbx"))
	{
		Logger::Error("Failed to load puck model!");
	}

	ModelRenderer* renderer = AddComponent<ModelRenderer>();
	renderer->SetModel(m_Model);
	renderer->SetShader(shader);

	// モデルのサイズに合わせてスケール調整
	GetTransform()->scale = Vector3(0.5f, 0.5f, 0.5f);

	// コライダー設定
	m_Collider = AddComponent<CircleCollider>();
	m_Collider->radius = 0.49f; // モデルの半径に合わせて調整

	//--------------------------------------------------------------------------
	// トレイル (リボン軌跡) の設定
	//--------------------------------------------------------------------------
	m_Trail = AddComponent<TrailRenderer>();

	// テクスチャロード
	m_Trail->Init("Asset/Texture/Trail_alpha.png", 1.f, 0.7f);

	// 色の設定 (鮮やかな水色 -> 透明)
	m_Trail->SetColor(
		DirectX::XMFLOAT4(0.2f, 0.8f, 1.0f, 1.f), // 始点
		DirectX::XMFLOAT4(0.0f, 0.2f, 1.0f, 0.5f)  // 終点
	);

	// 滑らかさを出すために、頂点生成の最小距離を小さく設定する
	m_Trail->SetMinVertexDistance(0.01f);

	//--------------------------------------------------------------------------
	// 火花エフェクト (ParticleEmitter) の設定
	//--------------------------------------------------------------------------
	m_SparkShader = new ParticleShader();
	m_SparkShader->Load(L"Shader/ParticleLineVertexShader.hlsl", L"Shader/ParticlePixelShader.hlsl");

	m_Sparks = AddComponent<ParticleEmitter>();
	m_Sparks->Init(200);

	// 自分で作ったシェーダーをセット
	m_Sparks->SetShader(m_SparkShader);

	m_Sparks->SetTexture(Texture::Load("Asset/Texture/white.png"));

	// パラメータ設定
	m_Sparks->settings.emitRate = 0.0f;
	m_Sparks->settings.lifeMin = 0.08f;
	m_Sparks->settings.lifeMax = 0.1f;
	m_Sparks->settings.sizeStart = 0.3f;
	m_Sparks->settings.sizeEnd = 0.1f;
	m_Sparks->settings.spread = 0.f; // 完全に水平に飛ばす
	//m_Sparks->settings.additiveBlend = false; // 加算ブレンドはオフにして通常のアルファブレンドにする
	m_Sparks->settings.colorStart = DirectX::XMFLOAT4(1.0f, 0.64f, 0.1f, 1.0f);
	m_Sparks->settings.colorEnd = DirectX::XMFLOAT4(1.0f, 0.64f, 0.1f, 1.0f);
}

//==============================================================================
// 更新
//==============================================================================
void Puck::Update(float deltaTime)
{
	// 移動計算
	Transform* transform = GetTransform();
	transform->position.x += m_VelX * deltaTime;
	transform->position.z += m_VelZ * deltaTime;

	// 物理挙動・補正（摩擦・壁反射）
	ApplyFriction();
	ReflectWalls();

	GameObject::Update(deltaTime);
}

//==============================================================================
// 速度設定
//==============================================================================
void Puck::SetVelocity(float vx, float vz)
{
	m_VelX = vx;
	m_VelZ = vz;
}

//==============================================================================
// 速度取得
//==============================================================================
void Puck::GetVelocity(float& outVX, float& outVZ) const
{
	outVX = m_VelX;
	outVZ = m_VelZ;
}

//==============================================================================
// 力を加える
//==============================================================================
void Puck::Push(float pushX, float pushZ)
{
	Transform* transform = GetTransform();
	transform->position.x += pushX;
	transform->position.z += pushZ;
}

//==============================================================================
// 摩擦
//==============================================================================
void Puck::ApplyFriction()
{
	m_VelX *= friction;
	m_VelZ *= friction;

	// 最小速度以下なら停止
	float speed = std::sqrt(m_VelX * m_VelX + m_VelZ * m_VelZ);
	if (speed < minSpeed)
	{
		m_VelX = 0.0f;
		m_VelZ = 0.0f;
	}
}

//==============================================================================
// 壁反射
//==============================================================================
void Puck::ReflectWalls()
{
	Transform* transform = GetTransform();
	float posX = transform->position.x;
	float posZ = transform->position.z;
	float radius = m_Collider->radius;

	// ゴールの範囲
	float goalHalfHeight = 4.0f;

	bool hit = false;
	Vector3 hitPos = transform->position;
	Vector3 hitNormal = Vector3(0, 0, 0);

	// 上壁
	if (posZ - radius < FieldBounds::TOP)
	{
		posZ = FieldBounds::TOP + radius;
		m_VelZ = -m_VelZ * restitution;

		hit = true;
		hitPos.z = FieldBounds::TOP; // 壁の位置
		hitNormal = Vector3(0, 0, 1); // 跳ね返る方向
	}
	// 下壁
	else if (posZ + radius > FieldBounds::BOTTOM)
	{
		posZ = FieldBounds::BOTTOM - radius;
		m_VelZ = -m_VelZ * restitution;

		hit = true;
		hitPos.z = FieldBounds::BOTTOM;
		hitNormal = Vector3(0, 0, -1);
	}

	// 左壁（ゴール以外）
	if (posX - radius < FieldBounds::LEFT)
	{
		if (posZ < -goalHalfHeight || posZ > goalHalfHeight)
		{
			posX = FieldBounds::LEFT + radius;
			m_VelX = -m_VelX * restitution;

			hit = true;
			hitPos.x = FieldBounds::LEFT;
			hitNormal = Vector3(1, 0, 0);
		}
	}

	// 右壁（ゴール以外）
	if (posX + radius > FieldBounds::RIGHT)
	{
		if (posZ < -goalHalfHeight || posZ > goalHalfHeight)
		{
			posX = FieldBounds::RIGHT - radius;
			m_VelX = -m_VelX * restitution;

			hit = true;
			hitPos.x = FieldBounds::RIGHT;
			hitNormal = Vector3(-1, 0, 0);
		}
	}

	transform->position.x = posX;
	transform->position.z = posZ;

	// 衝突していたら火花を散らす
	if (hit && m_Sparks)
	{
		// 衝突点（パックの中心から半径分ずらした位置）
		Vector3 spawnPos = transform->position - (hitNormal * radius);
		spawnPos.y = 0.5f; // 床より少し上

		// 火花を発生させる
		m_Sparks->Burst(7, spawnPos, hitNormal, 15.0f, 0.5f);
	}
}
