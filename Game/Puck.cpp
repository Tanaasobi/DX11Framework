//==============================================================================
// Puck.cpp - パック実装
//==============================================================================

#include "Puck.h"
#include "Field.h"
#include "Core/Graphics/MeshRenderer.h"
#include "Core/Graphics/Texture.h"
#include "Core/Physics/Collider.h"
#include "Core/Physics/Collision.h"
#include "Core/Graphics/Particle/ParticleEmitter.h"
#include "Core/Graphics/Particle/ParticleShader.h"
#include <cmath>

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
}

//==============================================================================
// 初期化
//==============================================================================
void Puck::Init(IShader* shader, ParticleShader* particleShader)
{
	// メッシュ追加
	MeshRenderer* renderer = AddComponent<MeshRenderer>();
	renderer->CreateCube(0.6f);
	renderer->SetShader(shader);

	// マテリアル（青っぽく）
	MATERIAL mat = {};
	mat.Ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.3f, 1.0f);
	mat.Diffuse = DirectX::XMFLOAT4(0.3f, 0.5f, 1.0f, 1.0f);
	mat.Specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mat.Emission = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mat.Shininess = 20.0f;
	mat.TextureEnable = FALSE;
	renderer->SetMaterial(mat);

	// コライダー追加
	m_Collider = AddComponent<CircleCollider>();
	m_Collider->radius = 0.3f;

	// パーティクルトレイル（雷風）
	if (particleShader)
	{
		m_TrailEmitter = AddComponent<ParticleEmitter>();
		m_TrailEmitter->Init(500);
		m_TrailEmitter->SetShader(particleShader);
		m_TrailEmitter->SetTexture("Asset/Texture/particle.png");

		// 雷っぽいパラメータ
		m_TrailEmitter->settings.emitRate = 300.0f;
		m_TrailEmitter->settings.lifeMin = 0.05f;
		m_TrailEmitter->settings.lifeMax = 0.2f;
		m_TrailEmitter->settings.sizeStart = 0.5f;
		m_TrailEmitter->settings.sizeEnd = 0.0f;

		// ジグザグ感を出すためにランダム速度を大きく
		m_TrailEmitter->settings.velocityMin = Vector3(-4.0f, -1.0f, -4.0f);
		m_TrailEmitter->settings.velocityMax = Vector3(4.0f, 2.0f, 4.0f);
		m_TrailEmitter->settings.acceleration = Vector3(0, 0, 0);

		// 雷の色（白〜青紫）
		m_TrailEmitter->settings.colorStart = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_TrailEmitter->settings.colorEnd = { 0.3f, 0.5f, 1.0f, 0.0f };

		// 加算ブレンドで光らせる
		m_TrailEmitter->settings.additiveBlend = true;

		// 回転でキラキラ感
		m_TrailEmitter->settings.rotationMin = 0.0f;
		m_TrailEmitter->settings.rotationMax = 360.0f;
		m_TrailEmitter->settings.rotationSpeedMin = -720.0f;
		m_TrailEmitter->settings.rotationSpeedMax = 720.0f;

		m_TrailEmitter->Stop();
	}

	// 初期位置
	GetTransform()->position = Vector3(0.0f, 0.3f, 0.0f);
}

//==============================================================================
// 更新
//==============================================================================
void Puck::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	// 速度適用
	Transform* transform = GetTransform();
	transform->position.x += m_VelX * deltaTime;
	transform->position.z += m_VelZ * deltaTime;

	// 摩擦適用
	ApplyFriction();

	// 壁反射
	ReflectWalls();

	// トレイル制御
	if (m_TrailEmitter)
	{
		float speed = std::sqrt(m_VelX * m_VelX + m_VelZ * m_VelZ);

		if (speed > 1.0f)
		{
			m_TrailEmitter->Play();

			// 速度に応じてエフェクト調整
			m_TrailEmitter->settings.emitRate = 100.0f + speed * 30.0f;
			m_TrailEmitter->settings.sizeStart = 0.3f + speed * 0.05f;

			// 速いほどジグザグ強く
			float jitter = speed * 0.5f;
			m_TrailEmitter->settings.velocityMin = Vector3(-jitter, -jitter * 0.3f, -jitter);
			m_TrailEmitter->settings.velocityMax = Vector3(jitter, jitter * 0.5f, jitter);
		}
		else
		{
			m_TrailEmitter->Stop();
		}
	}
}

//==============================================================================
// 摩擦適用
//==============================================================================
void Puck::ApplyFriction()
{
	m_VelX *= friction;
	m_VelZ *= friction;

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
// 押し出し
//==============================================================================
void Puck::Push(float pushX, float pushZ)
{
	Transform* transform = GetTransform();
	transform->position.x += pushX;
	transform->position.z += pushZ;
}
