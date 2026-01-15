#pragma once

//==============================================================================
// Scene.h - シーンクラス
//==============================================================================

#include "../System/main.h"
#include "../Object/GameObject.h"

//==============================================================================
// Scene クラス
// - シーン単位でのGameObject管理
// - 更新・描画の一括呼び出し
//==============================================================================
class Scene
{
public:
    Scene(const std::string& name = "Scene");
    virtual ~Scene();

    // コピー禁止
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    //--------------------------------------------------------------------------
    // ライフサイクル（派生クラスでオーバーライド可能）
    //--------------------------------------------------------------------------
    virtual void Init() {}      // シーン初期化
    virtual void Uninit() {}    // シーン終了
    virtual void Update(float deltaTime);
    virtual void LateUpdate(float deltaTime);
    virtual void Render();

    //--------------------------------------------------------------------------
    // GameObject管理
    //--------------------------------------------------------------------------
    GameObject* CreateGameObject(const std::string& name = "GameObject");
	void AddGameObject(GameObject* gameObject);
    void DestroyGameObject(GameObject* gameObject);
    GameObject* FindGameObject(const std::string& name);
	template<typename T> T* FindGameObjectOfType();

    //--------------------------------------------------------------------------
    // アクセサ
    //--------------------------------------------------------------------------
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

protected:
    std::string m_Name;
    std::vector<std::unique_ptr<GameObject>> m_GameObjects;

    // 削除予定のオブジェクト（フレーム終了時に削除）
    std::vector<GameObject*> m_DestroyQueue;

    void ProcessDestroyQueue();
};

//==============================================================================
// テンプレート実装
//==============================================================================
//==============================================================================
// GameObject検索(型)
//==============================================================================
template<typename T>
T* Scene::FindGameObjectOfType()
{
	for (auto& obj : m_GameObjects)
	{
		T* casted = dynamic_cast<T*>(obj.get());
		if (casted)
		{
			return casted;
		}
	}
	return nullptr;
}
