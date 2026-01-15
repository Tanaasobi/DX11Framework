#pragma once

//==============================================================================
// GameObject.h - ゲームオブジェクトクラス
//==============================================================================

#include "../System/main.h"
#include "Component.h"
#include "Transform.h"

//==============================================================================
// GameObject クラス
// - コンポーネントを持つオブジェクト
// - 親子関係をサポート
//==============================================================================
class GameObject
{
public:
    GameObject(const std::string& name = "GameObject");
    ~GameObject();

    // コピー禁止
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    //--------------------------------------------------------------------------
    // ライフサイクル
    //--------------------------------------------------------------------------
    virtual void Update(float deltaTime);
    virtual void LateUpdate(float deltaTime);
    void Render();

    //--------------------------------------------------------------------------
    // コンポーネント操作
    //--------------------------------------------------------------------------
    template<typename T>
    T* AddComponent();

    template<typename T>
    T* GetComponent();

    template<typename T>
    bool HasComponent();

    //--------------------------------------------------------------------------
    // 親子関係
    //--------------------------------------------------------------------------
    void SetParent(GameObject* parent);
    GameObject* GetParent() const { return m_Parent; }
    const std::vector<GameObject*>& GetChildren() const { return m_Children; }

    //--------------------------------------------------------------------------
    // アクセサ
    //--------------------------------------------------------------------------
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    bool IsEnabled() const { return m_Enabled; }
    void SetEnabled(bool enabled) { m_Enabled = enabled; }

    Transform* GetTransform() const { return m_Transform; }

private:
    std::string m_Name;
    bool        m_Enabled = true;

    Transform* m_Transform = nullptr;
    std::vector<std::unique_ptr<Component>> m_Components;

    GameObject* m_Parent = nullptr;
    std::vector<GameObject*> m_Children;

    void AddChild(GameObject* child);
    void RemoveChild(GameObject* child);
};

//==============================================================================
// テンプレート実装
//==============================================================================

template<typename T>
T* GameObject::AddComponent()
{
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

    auto component = std::make_unique<T>();
    T* ptr = component.get();

    ptr->SetGameObject(this);
    ptr->OnAttach();

    m_Components.push_back(std::move(component));

    return ptr;
}

template<typename T>
T* GameObject::GetComponent()
{
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

    for (auto& comp : m_Components)
    {
        T* casted = dynamic_cast<T*>(comp.get());
        if (casted)
            return casted;
    }

    return nullptr;
}

template<typename T>
bool GameObject::HasComponent()
{
    return GetComponent<T>() != nullptr;
}
