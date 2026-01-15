//==============================================================================
// GameObject.cpp - ゲームオブジェクトクラス実装
//==============================================================================

#include "GameObject.h"
#include "../System/Logger.h"

//==============================================================================
// コンストラクタ
//==============================================================================
GameObject::GameObject(const std::string& name)
    : m_Name(name)
    , m_Enabled(true)
    , m_Parent(nullptr)
{
    // Transformは必ず持つ
    m_Transform = AddComponent<Transform>();
}

//==============================================================================
// デストラクタ
//==============================================================================
GameObject::~GameObject()
{
    // 子オブジェクトの親参照をクリア
    for (auto* child : m_Children)
    {
        child->m_Parent = nullptr;
    }
    m_Children.clear();

    // コンポーネントのデタッチ
    for (auto& comp : m_Components)
    {
        comp->OnDetach();
    }
    m_Components.clear();
}

//==============================================================================
// 更新
//==============================================================================
void GameObject::Update(float deltaTime)
{
    if (!m_Enabled) return;

    for (auto& comp : m_Components)
    {
        if (!comp->IsEnabled()) continue;

        // 初回はStartを呼ぶ
        if (!comp->m_Started)
        {
            comp->Start();
            comp->m_Started = true;
        }

        comp->Update(deltaTime);
    }
}

//==============================================================================
// 後処理更新
//==============================================================================
void GameObject::LateUpdate(float deltaTime)
{
    if (!m_Enabled) return;

    for (auto& comp : m_Components)
    {
        if (!comp->IsEnabled()) continue;
        comp->LateUpdate(deltaTime);
    }
}

//==============================================================================
// 描画
//==============================================================================
void GameObject::Render()
{
    if (!m_Enabled) return;

    for (auto& comp : m_Components)
    {
        if (!comp->IsEnabled()) continue;
        comp->Render();
    }
}

//==============================================================================
// 親子関係
//==============================================================================
void GameObject::SetParent(GameObject* parent)
{
    // 現在の親から削除
    if (m_Parent)
    {
        m_Parent->RemoveChild(this);
    }

    m_Parent = parent;

    // 新しい親に追加
    if (m_Parent)
    {
        m_Parent->AddChild(this);
    }
}

void GameObject::AddChild(GameObject* child)
{
    m_Children.push_back(child);
}

void GameObject::RemoveChild(GameObject* child)
{
    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end())
    {
        m_Children.erase(it);
    }
}
