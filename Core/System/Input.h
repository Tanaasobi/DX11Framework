#pragma once

//==============================================================================
// Input.h - 入力管理クラス
//==============================================================================

#include "main.h"
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

//==============================================================================
// キーコード
//==============================================================================
enum class KeyCode
{
    // アルファベット
    A = 'A', B = 'B', C = 'C', D = 'D', E = 'E', F = 'F', G = 'G',
    H = 'H', I = 'I', J = 'J', K = 'K', L = 'L', M = 'M', N = 'N',
    O = 'O', P = 'P', Q = 'Q', R = 'R', S = 'S', T = 'T', U = 'U',
    V = 'V', W = 'W', X = 'X', Y = 'Y', Z = 'Z',

    // 数字
    Num0 = '0', Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4',
    Num5 = '5', Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',

    // ファンクションキー
    F1 = VK_F1, F2 = VK_F2, F3 = VK_F3, F4 = VK_F4,
    F5 = VK_F5, F6 = VK_F6, F7 = VK_F7, F8 = VK_F8,
    F9 = VK_F9, F10 = VK_F10, F11 = VK_F11, F12 = VK_F12,

    // 特殊キー
    Space = VK_SPACE,
    Enter = VK_RETURN,
    Escape = VK_ESCAPE,
    Tab = VK_TAB,
    Backspace = VK_BACK,
    Delete = VK_DELETE,
    Insert = VK_INSERT,

    // 矢印キー
    Left = VK_LEFT,
    Right = VK_RIGHT,
    Up = VK_UP,
    Down = VK_DOWN,

    // 修飾キー
    LeftShift = VK_LSHIFT,
    RightShift = VK_RSHIFT,
    LeftCtrl = VK_LCONTROL,
    RightCtrl = VK_RCONTROL,
    LeftAlt = VK_LMENU,
    RightAlt = VK_RMENU,

    // マウスボタン
    MouseLeft = VK_LBUTTON,
    MouseRight = VK_RBUTTON,
    MouseMiddle = VK_MBUTTON,
};

//==============================================================================
// ゲームパッドボタン
//==============================================================================
enum class GamepadButton
{
    DPadUp = XINPUT_GAMEPAD_DPAD_UP,
    DPadDown = XINPUT_GAMEPAD_DPAD_DOWN,
    DPadLeft = XINPUT_GAMEPAD_DPAD_LEFT,
    DPadRight = XINPUT_GAMEPAD_DPAD_RIGHT,
    Start = XINPUT_GAMEPAD_START,
    Back = XINPUT_GAMEPAD_BACK,
    LeftThumb = XINPUT_GAMEPAD_LEFT_THUMB,
    RightThumb = XINPUT_GAMEPAD_RIGHT_THUMB,
    LeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
    RightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
    A = XINPUT_GAMEPAD_A,
    B = XINPUT_GAMEPAD_B,
    X = XINPUT_GAMEPAD_X,
    Y = XINPUT_GAMEPAD_Y,
};

//==============================================================================
// ゲームパッド状態
//==============================================================================
struct GamepadState
{
    bool  connected;
    WORD  buttons;
    WORD  prevButtons;
    float leftTrigger;
    float rightTrigger;
    float leftStickX;
    float leftStickY;
    float rightStickX;
    float rightStickY;
};

//==============================================================================
// Input クラス
//==============================================================================
class Input
{
public:
    //--------------------------------------------------------------------------
    // 初期化・終了・更新
    //--------------------------------------------------------------------------
    static void Init(HWND hWnd);
    static void Uninit();
    static void Update();

    //--------------------------------------------------------------------------
    // キーボード
    //--------------------------------------------------------------------------
    static bool GetKey(KeyCode key);           // 押している間
    static bool GetKeyDown(KeyCode key);       // 押した瞬間
    static bool GetKeyUp(KeyCode key);         // 離した瞬間

    //--------------------------------------------------------------------------
    // マウス
    //--------------------------------------------------------------------------
    static void GetMousePosition(int& x, int& y);
    static void GetMouseDelta(int& dx, int& dy);
    static float GetMouseWheel();

    // マウスホイールのメッセージ処理用（WndProcから呼び出す）
    static void SetMouseWheel(float delta);

    //--------------------------------------------------------------------------
    // ゲームパッド（最大4台）
    //--------------------------------------------------------------------------
    static bool IsGamepadConnected(int index);
    static bool GetGamepadButton(int index, GamepadButton button);
    static bool GetGamepadButtonDown(int index, GamepadButton button);
    static bool GetGamepadButtonUp(int index, GamepadButton button);
    static float GetGamepadLeftTrigger(int index);
    static float GetGamepadRightTrigger(int index);
    static void GetGamepadLeftStick(int index, float& x, float& y);
    static void GetGamepadRightStick(int index, float& x, float& y);

private:
    static HWND m_hWnd;

    // キーボード
    static BYTE m_KeyStates[256];
    static BYTE m_PrevKeyStates[256];

    // マウス
    static int   m_MouseX;
    static int   m_MouseY;
    static int   m_PrevMouseX;
    static int   m_PrevMouseY;
    static float m_MouseWheel;

    // ゲームパッド
    static constexpr int MAX_GAMEPADS = 4;
    static GamepadState m_Gamepads[MAX_GAMEPADS];

    // デッドゾーン処理
    static float ApplyDeadzone(float value, float deadzone);
};