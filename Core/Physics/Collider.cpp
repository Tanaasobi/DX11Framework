//==============================================================================
// Collider.cpp - コライダーコンポーネント実装
//==============================================================================

#include "Collider.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"

//==============================================================================
// Collider 基底クラス
//==============================================================================
Collider::Collider()
{
}

Collider::~Collider()
{
}

void Collider::Update(float deltaTime)
{
	UpdateWorldPosition();
}

void Collider::UpdateWorldPosition()
{
	Transform* transform = GetGameObject()->GetTransform();

	// XZ平面で動作（Yは上方向）
	m_WorldX = transform->position.x + offset.x;
	m_WorldY = transform->position.z + offset.z;
}

//==============================================================================
// CircleCollider
//==============================================================================
CircleCollider::CircleCollider()
{
}

CircleCollider::~CircleCollider()
{
}

//==============================================================================
// BoxCollider
//==============================================================================
BoxCollider::BoxCollider()
{
}

BoxCollider::~BoxCollider()
{
}
