//==============================================================================
// Collision.cpp - 当たり判定ユーティリティ実装
//==============================================================================

#include "Collision.h"
#include <cmath>
#include <algorithm>

//==============================================================================
// コライダー同士の判定（タイプに応じて振り分け）
//==============================================================================
bool Collision::Check(Collider* a, Collider* b)
{
	if (!a || !b) return false;

	ColliderType typeA = a->GetType();
	ColliderType typeB = b->GetType();

	if (typeA == ColliderType::Circle && typeB == ColliderType::Circle)
	{
		return CircleVsCircle(static_cast<CircleCollider*>(a), static_cast<CircleCollider*>(b));
	}
	else if (typeA == ColliderType::Circle && typeB == ColliderType::Box)
	{
		return CircleVsBox(static_cast<CircleCollider*>(a), static_cast<BoxCollider*>(b));
	}
	else if (typeA == ColliderType::Box && typeB == ColliderType::Circle)
	{
		return CircleVsBox(static_cast<CircleCollider*>(b), static_cast<BoxCollider*>(a));
	}

	return false;
}

//==============================================================================
// 円 vs 円
//==============================================================================
bool Collision::CircleVsCircle(CircleCollider* a, CircleCollider* b)
{
	float pushX, pushY;
	return CircleVsCircle(a, b, pushX, pushY);
}

//==============================================================================
// 円 vs 円（押し戻し付き）
//==============================================================================
bool Collision::CircleVsCircle(CircleCollider* a, CircleCollider* b,
	float& pushX, float& pushY)
{
	if (!a || !b)
	{
		pushX = pushY = 0.0f;
		return false;
	}

	float dx = b->GetX() - a->GetX();
	float dy = b->GetY() - a->GetY();
	float distSq = dx * dx + dy * dy;
	float radiusSum = a->GetRadius() + b->GetRadius();

	if (distSq <= radiusSum * radiusSum)
	{
		float dist = std::sqrt(distSq);
		if (dist > 0.0001f)
		{
			float overlap = radiusSum - dist;
			pushX = (dx / dist) * overlap;
			pushY = (dy / dist) * overlap;
		}
		else
		{
			pushX = radiusSum;
			pushY = 0.0f;
		}
		return true;
	}

	pushX = pushY = 0.0f;
	return false;
}

//==============================================================================
// 円 vs 矩形
//==============================================================================
bool Collision::CircleVsBox(CircleCollider* circle, BoxCollider* box)
{
	float pushX, pushY;
	return CircleVsBox(circle, box, pushX, pushY);
}

//==============================================================================
// 円 vs 矩形（押し戻し付き）
//==============================================================================
bool Collision::CircleVsBox(CircleCollider* circle, BoxCollider* box,
	float& pushX, float& pushY)
{
	if (!circle || !box)
	{
		pushX = pushY = 0.0f;
		return false;
	}

	// 矩形上の最近接点を求める
	float closestX = std::clamp(circle->GetX(), box->Left(), box->Right());
	float closestY = std::clamp(circle->GetY(), box->Top(), box->Bottom());

	// 円の中心と最近接点の距離
	float dx = circle->GetX() - closestX;
	float dy = circle->GetY() - closestY;
	float distSq = dx * dx + dy * dy;
	float radius = circle->GetRadius();

	if (distSq <= radius * radius)
	{
		float dist = std::sqrt(distSq);
		if (dist > 0.0001f)
		{
			float overlap = radius - dist;
			pushX = (dx / dist) * overlap;
			pushY = (dy / dist) * overlap;
		}
		else
		{
			pushX = radius;
			pushY = 0.0f;
		}
		return true;
	}

	pushX = pushY = 0.0f;
	return false;
}

//==============================================================================
// 円の壁反射
//==============================================================================
void Collision::ReflectCircleInBounds(
	float& posX, float& posY,
	float& velX, float& velY,
	float radius,
	float fieldLeft, float fieldRight,
	float fieldTop, float fieldBottom,
	float restitution)
{
	// 左壁
	if (posX - radius < fieldLeft)
	{
		posX = fieldLeft + radius;
		velX = -velX * restitution;
	}
	// 右壁
	if (posX + radius > fieldRight)
	{
		posX = fieldRight - radius;
		velX = -velX * restitution;
	}
	// 上壁
	if (posY - radius < fieldTop)
	{
		posY = fieldTop + radius;
		velY = -velY * restitution;
	}
	// 下壁
	if (posY + radius > fieldBottom)
	{
		posY = fieldBottom - radius;
		velY = -velY * restitution;
	}
}
