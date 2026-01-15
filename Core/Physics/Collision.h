#pragma once

//==============================================================================
// Collision.h - 当たり判定ユーティリティ
//==============================================================================

#include "Collider.h"

//==============================================================================
// Collision クラス（静的ユーティリティ）
//==============================================================================
class Collision
{
public:
	// コライダー同士の判定
	static bool Check(Collider* a, Collider* b);

	// 円 vs 円
	static bool CircleVsCircle(CircleCollider* a, CircleCollider* b);

	// 円 vs 矩形
	static bool CircleVsBox(CircleCollider* circle, BoxCollider* box);

	// 円 vs 円（押し戻しベクトル付き）
	static bool CircleVsCircle(CircleCollider* a, CircleCollider* b,
		float& pushX, float& pushY);

	// 円 vs 矩形（押し戻しベクトル付き）
	static bool CircleVsBox(CircleCollider* circle, BoxCollider* box,
		float& pushX, float& pushY);

	// 円の壁反射（フィールド境界）
	static void ReflectCircleInBounds(
		float& posX, float& posY,
		float& velX, float& velY,
		float radius,
		float fieldLeft, float fieldRight,
		float fieldTop, float fieldBottom,
		float restitution = 1.0f
	);
};
