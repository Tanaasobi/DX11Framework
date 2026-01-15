//==============================================================================
// Scene.cpp - シーンクラス実装
//==============================================================================

#include "Scene.h"
#include "../System/Logger.h"

//==============================================================================
// コンストラクタ
//==============================================================================
Scene::Scene(const std::string& name)
    : m_Name(name)
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Scene::~Scene()
{
    m_DestroyQueue.clear();
    m_GameObjects.clear();
}

//==============================================================================
// 更新
//==============================================================================
void Scene::Update(float deltaTime)
{
    for (auto& go : m_GameObjects)
    {
        if (go)
        {
            go->Update(deltaTime);
        }
    }
}

//==============================================================================
// 後処理更新
//==============================================================================
void Scene::LateUpdate(float deltaTime)
{
    for (auto& go : m_GameObjects)
    {
        if (go)
        {
            go->LateUpdate(deltaTime);
        }
    }

    // 削除予定のオブジェクトを処理
    ProcessDestroyQueue();
}

//==============================================================================
// 描画
//==============================================================================
void Scene::Render()
{
    for (auto& go : m_GameObjects)
    {
        if (go)
        {
            go->Render();
        }
    }
}

//==============================================================================
// GameObject作成
//==============================================================================
GameObject* Scene::CreateGameObject(const std::string& name)
{
    auto go = std::make_unique<GameObject>(name);
    GameObject* ptr = go.get();
    m_GameObjects.push_back(std::move(go));

    Logger::InfoFormat("GameObject created: %s", name.c_str());

    return ptr;
}

//==============================================================================
// 外部で作成したGameObjectを追加
//==============================================================================
void Scene::AddGameObject(GameObject* gameObject)
{
	if (gameObject)
	{
		m_GameObjects.push_back(std::unique_ptr<GameObject>(gameObject));
	}
}

//==============================================================================
// GameObject削除（予約）
//==============================================================================
void Scene::DestroyGameObject(GameObject* gameObject)
{
    if (gameObject)
    {
        m_DestroyQueue.push_back(gameObject);
    }
}

//==============================================================================
// GameObject検索
//==============================================================================
GameObject* Scene::FindGameObject(const std::string& name)
{
    for (auto& go : m_GameObjects)
    {
        if (go && go->GetName() == name)
        {
            return go.get();
        }
    }
    return nullptr;
}

//==============================================================================
// 削除キューの処理
//==============================================================================
void Scene::ProcessDestroyQueue()
{
    for (auto* go : m_DestroyQueue)
    {
        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
            [go](const std::unique_ptr<GameObject>& ptr) {
                return ptr.get() == go;
            });

        if (it != m_GameObjects.end())
        {
            Logger::InfoFormat("GameObject destroyed: %s", (*it)->GetName().c_str());
            m_GameObjects.erase(it);
        }
    }
    m_DestroyQueue.clear();
}
