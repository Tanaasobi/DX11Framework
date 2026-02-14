//==============================================================================
// CameraShake.cpp - 実装
//==============================================================================

#include "CameraShake.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"
#include <cstdlib> // rand()用

void CameraShake::Shake(float duration, float magnitude)
{
	m_Timer = duration;
	m_Magnitude = magnitude;
}

void CameraShake::LateUpdate(float deltaTime)
{
	Transform* transform = m_GameObject->GetTransform();
	if (!transform) return;

	// 前のフレームでずらした分を戻す
	transform->position -= m_LastOffset;
	m_LastOffset = Vector3(0, 0, 0);

	// 2. まだ揺れる時間が残っていれば、新しくずらす
	if (m_Timer > 0.0f)
	{
		m_Timer -= deltaTime;

		if (m_Timer > 0.0f)
		{
			// -1.0 〜 1.0 のランダム値を作成
			float x = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * m_Magnitude;
			float y = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * m_Magnitude;

			// Z軸（奥行き）は揺らさないのが一般的
			Vector3 offset(x, y, 0.0f);

			// 座標に加算して、今回ずらした量を記録
			transform->position += offset;
			m_LastOffset = offset;
		}
	}
}
