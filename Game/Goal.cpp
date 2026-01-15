//==============================================================================
// Goal.cpp - ゴール実装
//==============================================================================

#include "Goal.h"
#include "Field.h"
#include "Core/Graphics/MeshRenderer.h"

//==============================================================================
// コンストラクタ
//==============================================================================
Goal::Goal(Team team)
	: GameObject(team == Team::Left ? "GoalLeft" : "GoalRight")
	, m_Team(team)
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Goal::~Goal()
{
}

//==============================================================================
// 初期化
//==============================================================================
void Goal::Init(IShader* shader)
{
	// メッシュ追加
	MeshRenderer* renderer = AddComponent<MeshRenderer>();
	renderer->CreateCube(1.0f);
	renderer->SetShader(shader);

	// マテリアル（チームカラー）
	MATERIAL mat = {};
	if (m_Team == Team::Left)
	{
		// 青チーム
		mat.Ambient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.3f, 1.0f);
		mat.Diffuse = DirectX::XMFLOAT4(0.2f, 0.4f, 1.0f, 0.5f);
	}
	else
	{
		// 赤チーム
		mat.Ambient = DirectX::XMFLOAT4(0.3f, 0.0f, 0.0f, 1.0f);
		mat.Diffuse = DirectX::XMFLOAT4(1.0f, 0.3f, 0.2f, 0.5f);
	}
	mat.Specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mat.Emission = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mat.Shininess = 10.0f;
	mat.TextureEnable = FALSE;
	renderer->SetMaterial(mat);

	// コライダー追加（矩形）
	m_Collider = AddComponent<BoxCollider>();
	m_Collider->width = WIDTH;
	m_Collider->height = HEIGHT;

	// 位置設定
	float xPos = (m_Team == Team::Left) ? FieldBounds::LEFT - WIDTH * 0.5f : FieldBounds::RIGHT + WIDTH * 0.5f;
	GetTransform()->position = Vector3(xPos, DEPTH * 0.5f, 0.0f);
	GetTransform()->scale = Vector3(WIDTH, DEPTH, HEIGHT);
}

//==============================================================================
// 更新
//==============================================================================
void Goal::Update(float deltaTime)
{
	GameObject::Update(deltaTime);
}
