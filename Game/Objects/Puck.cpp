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
	m_Trail->Init("Asset/Texture/Trail_alpha.png", 0.7f, 0.7f);

	// 色の設定 (鮮やかな水色 -> 透明)git 
	m_Trail->SetColor(
		DirectX::XMFLOAT4(0.2f, 0.8f, 1.0f, 0.8f), // 始点
		DirectX::XMFLOAT4(0.0f, 0.2f, 1.0f, 0.0f)  // 終点
	);

	// 滑らかさを出すために、頂点生成の最小距離を小さく設定する
	m_Trail->SetMinVertexDistance(0.01f);
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

	// ゴールの範囲（Goal::HEIGHT / 2）
	float goalHalfHeight = 4.0f;  // 8.0 / 2

	// 上壁
	if (posZ - radius < FieldBounds::TOP)
	{
		posZ = FieldBounds::TOP + radius;
		m_VelZ = -m_VelZ * restitution;
	}
	// 下壁
	if (posZ + radius > FieldBounds::BOTTOM)
	{
		posZ = FieldBounds::BOTTOM - radius;
		m_VelZ = -m_VelZ * restitution;
	}

	// 左壁（ゴール部分を除く）
	if (posX - radius < FieldBounds::LEFT)
	{
		if (posZ < -goalHalfHeight || posZ > goalHalfHeight)
		{
			posX = FieldBounds::LEFT + radius;
			m_VelX = -m_VelX * restitution;
		}
	}

	// 右壁（ゴール部分を除く）
	if (posX + radius > FieldBounds::RIGHT)
	{
		if (posZ < -goalHalfHeight || posZ > goalHalfHeight)
		{
			posX = FieldBounds::RIGHT - radius;
			m_VelX = -m_VelX * restitution;
		}
	}

	transform->position.x = posX;
	transform->position.z = posZ;
}
