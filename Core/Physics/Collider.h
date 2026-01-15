#pragma once

//==============================================================================
// Collider.h - コライダーコンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/Math/Vector3.h"

//==============================================================================
// コライダータイプ
//==============================================================================
enum class ColliderType
{
	Circle,
	Box
};

//==============================================================================
// Collider 基底クラス
//==============================================================================
class Collider : public Component
{
public:
	Collider();
	virtual ~Collider();

	void Update(float deltaTime) override;

	// タイプ取得
	virtual ColliderType GetType() const = 0;

	// 中心位置（ワールド座標）
	float GetX() const { return m_WorldX; }
	float GetY() const { return m_WorldY; }

	// オフセット（ローカル座標）
	Vector3 offset;

protected:
	float m_WorldX = 0.0f;
	float m_WorldY = 0.0f;

	void UpdateWorldPosition();
};

//==============================================================================
// CircleCollider - 円形コライダー
//==============================================================================
class CircleCollider : public Collider
{
public:
	CircleCollider();
	virtual ~CircleCollider();

	ColliderType GetType() const override { return ColliderType::Circle; }

	float GetRadius() const { return radius; }

	// パラメータ
	float radius = 0.5f;
};

//==============================================================================
// BoxCollider - 矩形コライダー（AABB）
//==============================================================================
class BoxCollider : public Collider
{
public:
	BoxCollider();
	virtual ~BoxCollider();

	ColliderType GetType() const override { return ColliderType::Box; }

	float GetWidth() const { return width; }
	float GetHeight() const { return height; }

	float Left()   const { return m_WorldX - width * 0.5f; }
	float Right()  const { return m_WorldX + width * 0.5f; }
	float Top()    const { return m_WorldY - height * 0.5f; }
	float Bottom() const { return m_WorldY + height * 0.5f; }

	// パラメータ
	float width = 1.0f;
	float height = 1.0f;
};
