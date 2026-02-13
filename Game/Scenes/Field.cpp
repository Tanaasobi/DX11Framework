//==============================================================================
// Field.cpp - フィールド実装
//==============================================================================

#include "Game/Objects/Field.h"
#include "Core/Graphics/MeshRenderer.h"

//==============================================================================
// コンストラクタ
//==============================================================================
Field::Field()
	: GameObject("Field")
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Field::~Field()
{
}

//==============================================================================
// 初期化
//==============================================================================
void Field::Init(IShader* shader)
{
	// メッシュ追加
	MeshRenderer* renderer = AddComponent<MeshRenderer>();
	renderer->CreateQuad(FieldBounds::WIDTH, FieldBounds::HEIGHT);
	renderer->SetShader(shader);

	// マテリアル（緑色）
	MATERIAL mat = {};
	mat.Ambient = DirectX::XMFLOAT4(0.1f, 0.3f, 0.1f, 1.0f);
	mat.Diffuse = DirectX::XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
	mat.Specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mat.Emission = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mat.Shininess = 0.0f;
	mat.TextureEnable = FALSE;
	renderer->SetMaterial(mat);

	// 床を水平に（XZ平面）
	GetTransform()->SetEulerAngles(90.0f, 0.0f, 0.0f);
}
