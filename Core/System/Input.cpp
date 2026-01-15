//==============================================================================
// Input.cpp - 入力管理クラス実装
//==============================================================================

#include "Input.h"

//==============================================================================
// 静的メンバ変数の定義
//==============================================================================
HWND Input::m_hWnd = nullptr;

// キーボード
BYTE Input::m_KeyStates[256] = {};
BYTE Input::m_PrevKeyStates[256] = {};

// マウス
int   Input::m_MouseX = 0;
int   Input::m_MouseY = 0;
int   Input::m_PrevMouseX = 0;
int   Input::m_PrevMouseY = 0;
float Input::m_MouseWheel = 0.0f;

// ゲームパッド
GamepadState Input::m_Gamepads[MAX_GAMEPADS] = {};

//==============================================================================
// 初期化
//==============================================================================
void Input::Init(HWND hWnd)
{
    m_hWnd = hWnd;

    // キーボード状態を初期化
    memset(m_KeyStates, 0, sizeof(m_KeyStates));
    memset(m_PrevKeyStates, 0, sizeof(m_PrevKeyStates));

    // マウス位置を初期化
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(m_hWnd, &pt);
    m_MouseX = m_PrevMouseX = pt.x;
    m_MouseY = m_PrevMouseY = pt.y;
    m_MouseWheel = 0.0f;

    // ゲームパッドを初期化
    memset(m_Gamepads, 0, sizeof(m_Gamepads));
}

//==============================================================================
// 終了処理
//==============================================================================
void Input::Uninit()
{
    // 特に解放するものはない
}

//==============================================================================
// 更新（毎フレーム呼び出し）
//==============================================================================
void Input::Update()
{
    //--------------------------------------------------------------------------
    // キーボード更新
    //--------------------------------------------------------------------------
    memcpy(m_PrevKeyStates, m_KeyStates, sizeof(m_KeyStates));

    for (int i = 0; i < 256; i++)
    {
        SHORT state = GetAsyncKeyState(i);
        m_KeyStates[i] = (state & 0x8000) ? 1 : 0;
    }

    //--------------------------------------------------------------------------
    // マウス更新
    //--------------------------------------------------------------------------
    m_PrevMouseX = m_MouseX;
    m_PrevMouseY = m_MouseY;

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(m_hWnd, &pt);
    m_MouseX = pt.x;
    m_MouseY = pt.y;

    // ホイールは毎フレームリセット（WndProcで設定される）
    m_MouseWheel = 0.0f;

    //--------------------------------------------------------------------------
    // ゲームパッド更新
    //--------------------------------------------------------------------------
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        // 前フレームのボタン状態を保存
        m_Gamepads[i].prevButtons = m_Gamepads[i].buttons;

        XINPUT_STATE state;
        DWORD result = XInputGetState(i, &state);

        if (result == ERROR_SUCCESS)
        {
            m_Gamepads[i].connected = true;
            m_Gamepads[i].buttons = state.Gamepad.wButtons;

            // トリガー（0.0〜1.0に正規化）
            m_Gamepads[i].leftTrigger = state.Gamepad.bLeftTrigger / 255.0f;
            m_Gamepads[i].rightTrigger = state.Gamepad.bRightTrigger / 255.0f;

            // スティック（-1.0〜1.0に正規化、デッドゾーン適用）
            constexpr float STICK_MAX = 32767.0f;
            constexpr float DEADZONE = 0.2f;

            float lx = state.Gamepad.sThumbLX / STICK_MAX;
            float ly = state.Gamepad.sThumbLY / STICK_MAX;
            float rx = state.Gamepad.sThumbRX / STICK_MAX;
            float ry = state.Gamepad.sThumbRY / STICK_MAX;

            m_Gamepads[i].leftStickX = ApplyDeadzone(lx, DEADZONE);
            m_Gamepads[i].leftStickY = ApplyDeadzone(ly, DEADZONE);
            m_Gamepads[i].rightStickX = ApplyDeadzone(rx, DEADZONE);
            m_Gamepads[i].rightStickY = ApplyDeadzone(ry, DEADZONE);
        }
        else
        {
            m_Gamepads[i].connected = false;
        }
    }
}

//==============================================================================
// キーボード
//==============================================================================
bool Input::GetKey(KeyCode key)
{
    return m_KeyStates[static_cast<int>(key)] != 0;
}

bool Input::GetKeyDown(KeyCode key)
{
    int k = static_cast<int>(key);
    return m_KeyStates[k] && !m_PrevKeyStates[k];
}

bool Input::GetKeyUp(KeyCode key)
{
    int k = static_cast<int>(key);
    return !m_KeyStates[k] && m_PrevKeyStates[k];
}

//==============================================================================
// マウス
//==============================================================================
void Input::GetMousePosition(int& x, int& y)
{
    x = m_MouseX;
    y = m_MouseY;
}

void Input::GetMouseDelta(int& dx, int& dy)
{
    dx = m_MouseX - m_PrevMouseX;
    dy = m_MouseY - m_PrevMouseY;
}

float Input::GetMouseWheel()
{
    return m_MouseWheel;
}

void Input::SetMouseWheel(float delta)
{
    m_MouseWheel = delta;
}

//==============================================================================
// ゲームパッド
//==============================================================================
bool Input::IsGamepadConnected(int index)
{
    if (index < 0 || index >= MAX_GAMEPADS) return false;
    return m_Gamepads[index].connected;
}

bool Input::GetGamepadButton(int index, GamepadButton button)
{
    if (index < 0 || index >= MAX_GAMEPADS) return false;
    if (!m_Gamepads[index].connected) return false;

    return (m_Gamepads[index].buttons & static_cast<WORD>(button)) != 0;
}

bool Input::GetGamepadButtonDown(int index, GamepadButton button)
{
    if (index < 0 || index >= MAX_GAMEPADS) return false;
    if (!m_Gamepads[index].connected) return false;

    WORD btn = static_cast<WORD>(button);
    bool current = (m_Gamepads[index].buttons & btn) != 0;
    bool prev = (m_Gamepads[index].prevButtons & btn) != 0;

    return current && !prev;
}

bool Input::GetGamepadButtonUp(int index, GamepadButton button)
{
    if (index < 0 || index >= MAX_GAMEPADS) return false;
    if (!m_Gamepads[index].connected) return false;

    WORD btn = static_cast<WORD>(button);
    bool current = (m_Gamepads[index].buttons & btn) != 0;
    bool prev = (m_Gamepads[index].prevButtons & btn) != 0;

    return !current && prev;
}

float Input::GetGamepadLeftTrigger(int index)
{
    if (index < 0 || index >= MAX_GAMEPADS) return 0.0f;
    if (!m_Gamepads[index].connected) return 0.0f;

    return m_Gamepads[index].leftTrigger;
}

float Input::GetGamepadRightTrigger(int index)
{
    if (index < 0 || index >= MAX_GAMEPADS) return 0.0f;
    if (!m_Gamepads[index].connected) return 0.0f;

    return m_Gamepads[index].rightTrigger;
}

void Input::GetGamepadLeftStick(int index, float& x, float& y)
{
    if (index < 0 || index >= MAX_GAMEPADS)
    {
        x = y = 0.0f;
        return;
    }
    if (!m_Gamepads[index].connected)
    {
        x = y = 0.0f;
        return;
    }

    x = m_Gamepads[index].leftStickX;
    y = m_Gamepads[index].leftStickY;
}

void Input::GetGamepadRightStick(int index, float& x, float& y)
{
    if (index < 0 || index >= MAX_GAMEPADS)
    {
        x = y = 0.0f;
        return;
    }
    if (!m_Gamepads[index].connected)
    {
        x = y = 0.0f;
        return;
    }

    x = m_Gamepads[index].rightStickX;
    y = m_Gamepads[index].rightStickY;
}

//==============================================================================
// デッドゾーン処理
//==============================================================================
float Input::ApplyDeadzone(float value, float deadzone)
{
    if (value > deadzone)
    {
        return (value - deadzone) / (1.0f - deadzone);
    }
    else if (value < -deadzone)
    {
        return (value + deadzone) / (1.0f - deadzone);
    }
    return 0.0f;
}