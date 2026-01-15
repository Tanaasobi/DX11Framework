//==============================================================================
// Time.cpp - 時間管理クラス実装
//==============================================================================

#include "Time.h"

//==============================================================================
// 静的メンバ変数の定義
//==============================================================================
LARGE_INTEGER Time::m_Frequency = {};
LARGE_INTEGER Time::m_LastTime = {};
LARGE_INTEGER Time::m_StartTime = {};

float Time::m_DeltaTime = 0.0f;
float Time::m_TotalTime = 0.0f;

float Time::m_FPS = 0.0f;
int   Time::m_FrameCount = 0;
float Time::m_FPSTimer = 0.0f;

//==============================================================================
// 初期化
//==============================================================================
void Time::Init()
{
    // タイマー周波数を取得
    QueryPerformanceFrequency(&m_Frequency);

    // 開始時間を記録
    QueryPerformanceCounter(&m_StartTime);
    m_LastTime = m_StartTime;

    m_DeltaTime = 0.0f;
    m_TotalTime = 0.0f;
    m_FPS = 0.0f;
    m_FrameCount = 0;
    m_FPSTimer = 0.0f;
}

//==============================================================================
// 更新（毎フレーム呼び出し）
//==============================================================================
void Time::Update()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    // deltaTimeを計算（秒単位）
    m_DeltaTime = static_cast<float>(currentTime.QuadPart - m_LastTime.QuadPart)
        / static_cast<float>(m_Frequency.QuadPart);

    // 極端に大きいdeltaTimeを制限（デバッグ時のブレークポイントなど対策）
    if (m_DeltaTime > 0.1f)
    {
        m_DeltaTime = 0.1f;
    }

    // 合計時間を更新
    m_TotalTime = static_cast<float>(currentTime.QuadPart - m_StartTime.QuadPart)
        / static_cast<float>(m_Frequency.QuadPart);

    // FPSを計算（1秒ごとに更新）
    m_FrameCount++;
    m_FPSTimer += m_DeltaTime;

    if (m_FPSTimer >= 1.0f)
    {
        m_FPS = static_cast<float>(m_FrameCount) / m_FPSTimer;
        m_FrameCount = 0;
        m_FPSTimer = 0.0f;
    }

    // 現在時間を保存
    m_LastTime = currentTime;
}