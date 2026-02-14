#pragma once

//==============================================================================
// CameraShake.h - カメラシェイク用コンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/Math/Vector3.h"

class CameraShake : public Component
{
public:
	// シェイク開始
	// duration: 揺れる時間（秒）
	// magnitude: 揺れの強さ（1.0fくらいが目安）
	void Shake(float duration, float magnitude);

	// ライフサイクル
	void LateUpdate(float deltaTime) override;

private:
	float m_Timer = 0.0f;           // 残り時間
	float m_Magnitude = 0.0f;       // 揺れの強さ
	Vector3 m_LastOffset;           // 前フレームでずらした量
};
