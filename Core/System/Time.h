#pragma once

//==============================================================================
// Time.h - 時間管理クラス
//==============================================================================

#include "main.h"

//==============================================================================
// Time クラス
// - フレーム間の経過時間（deltaTime）を計測
// - FPSの計算
// - 高精度タイマー（QueryPerformanceCounter）を使用
//==============================================================================
class Time
{
public:
	// 初期化
	static void Init();

	// フレーム開始時に呼び出し（deltaTimeを更新）
	static void Update();

	// 前フレームからの経過時間（秒）
	static float GetDeltaTime() { return m_DeltaTime; }

	// ゲーム開始からの経過時間（秒）
	static float GetTotalTime() { return m_TotalTime; }

	// 現在のFPS
	static float GetFPS() { return m_FPS; }

private:
	static LARGE_INTEGER m_Frequency;      // タイマー周波数
	static LARGE_INTEGER m_LastTime;       // 前フレームの時間
	static LARGE_INTEGER m_StartTime;      // ゲーム開始時の時間

	static float m_DeltaTime;              // フレーム間の経過時間
	static float m_TotalTime;              // 合計経過時間

	// FPS計算用
	static float m_FPS;
	static int   m_FrameCount;
	static float m_FPSTimer;
};
