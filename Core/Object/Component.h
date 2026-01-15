#pragma once

//==============================================================================
// Component.h - コンポーネント基底クラス
//==============================================================================

#include "../System/main.h"

// 前方宣言
class GameObject;

//==============================================================================
// Component クラス
// - 全てのコンポーネントの基底クラス
// - GameObjectにアタッチして機能を追加する
//==============================================================================
class Component
{
    friend class GameObject;

public:
    Component() = default;
    virtual ~Component() = default;

    // コピー禁止
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    //--------------------------------------------------------------------------
    // ライフサイクル（派生クラスでオーバーライド）
    //--------------------------------------------------------------------------
    virtual void OnAttach() {}                      // アタッチされた時
    virtual void OnDetach() {}                      // デタッチされる時
    virtual void Start() {}                         // 初回Update前に1度だけ
    virtual void Update(float deltaTime) {}         // 毎フレーム更新
    virtual void LateUpdate(float deltaTime) {}     // 後処理用更新
    virtual void Render() {}                        // 描画

    //--------------------------------------------------------------------------
    // アクセサ
    //--------------------------------------------------------------------------
    GameObject* GetGameObject() const { return m_GameObject; }
    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool enabled) { m_Enabled = enabled; }

protected:
    GameObject* m_GameObject = nullptr;
    bool        m_Enabled = true;
    bool        m_Started = false;

private:
    void SetGameObject(GameObject* gameObject) { m_GameObject = gameObject; }
};
