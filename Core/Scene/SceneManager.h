#pragma once

//==============================================================================
// SceneManager.h - シーン管理クラス
//==============================================================================

#include "../System/main.h"
#include "../System/Logger.h"
#include "Scene.h"

//==============================================================================
// SceneFactory 型定義
//==============================================================================
using SceneFactory = std::function<std::unique_ptr<Scene>()>;

//==============================================================================
// SceneManager クラス
// - 名前ベースでシーンを登録・切り替え
// - アクティブシーンを1つ管理
//==============================================================================
class SceneManager
{
public:
    // 初期化・終了
    static void Init();
    static void Uninit();

    // 更新・描画
    static void Update(float deltaTime);
    static void LateUpdate(float deltaTime);
    static void Render();

    // シーン登録
    template<typename T>
    static void RegisterScene(const std::string& sceneName);

    // シーン操作
    static void LoadScene(const std::string& sceneName);
    static void ReloadCurrentScene();
    static void UnloadCurrentScene();

    // アクセサ
    static Scene* GetActiveScene() { return m_ActiveScene.get(); }
    static const std::string& GetActiveSceneName() { return m_ActiveSceneName; }

private:
    static std::string m_ActiveSceneName;
    static std::unique_ptr<Scene> m_ActiveScene;
    static std::unordered_map<std::string, SceneFactory> m_SceneFactories;

    // 次のシーン（遅延ロード用）
    static std::string m_NextSceneName;
    static bool m_SceneChangeRequested;

    static void ProcessSceneChange();
};

//==============================================================================
// テンプレート実装
//==============================================================================
template<typename T>
void SceneManager::RegisterScene(const std::string& sceneName)
{
    static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");

    m_SceneFactories[sceneName] = []() {
        return std::make_unique<T>();
        };

    Logger::InfoFormat("Scene registered: %s", sceneName.c_str());
}
