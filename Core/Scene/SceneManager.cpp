//==============================================================================
// SceneManager.cpp - シーン管理クラス実装
//==============================================================================

#include "SceneManager.h"

//==============================================================================
// 静的メンバ変数の定義
//==============================================================================
std::string SceneManager::m_ActiveSceneName = "";
std::unique_ptr<Scene> SceneManager::m_ActiveScene = nullptr;
std::unordered_map<std::string, SceneFactory> SceneManager::m_SceneFactories;

std::string SceneManager::m_NextSceneName = "";
bool SceneManager::m_SceneChangeRequested = false;

//==============================================================================
// 初期化
//==============================================================================
void SceneManager::Init()
{
    m_ActiveSceneName = "";
    m_ActiveScene = nullptr;
    m_SceneFactories.clear();
    m_NextSceneName = "";
    m_SceneChangeRequested = false;

    Logger::Info("SceneManager initialized");
}

//==============================================================================
// 終了
//==============================================================================
void SceneManager::Uninit()
{
    UnloadCurrentScene();
    m_SceneFactories.clear();

    Logger::Info("SceneManager uninitialized");
}

//==============================================================================
// 更新
//==============================================================================
void SceneManager::Update(float deltaTime)
{
    // シーン切り替え処理
    ProcessSceneChange();

    if (m_ActiveScene)
    {
        m_ActiveScene->Update(deltaTime);
    }
}

//==============================================================================
// 後処理更新
//==============================================================================
void SceneManager::LateUpdate(float deltaTime)
{
    if (m_ActiveScene)
    {
        m_ActiveScene->LateUpdate(deltaTime);
    }
}

//==============================================================================
// 描画
//==============================================================================
void SceneManager::Render()
{
    if (m_ActiveScene)
    {
        m_ActiveScene->Render();
    }
}

//==============================================================================
// シーンロード（リクエスト）
//==============================================================================
void SceneManager::LoadScene(const std::string& sceneName)
{
    // ファクトリが登録されているか確認
    if (m_SceneFactories.find(sceneName) == m_SceneFactories.end())
    {
        Logger::ErrorFormat("Scene not found: %s", sceneName.c_str());
        return;
    }

    m_NextSceneName = sceneName;
    m_SceneChangeRequested = true;

    Logger::InfoFormat("Scene load requested: %s", sceneName.c_str());
}

//==============================================================================
// 現在のシーンをリロード
//==============================================================================
void SceneManager::ReloadCurrentScene()
{
    if (!m_ActiveSceneName.empty())
    {
        LoadScene(m_ActiveSceneName);
    }
}

//==============================================================================
// 現在のシーンをアンロード
//==============================================================================
void SceneManager::UnloadCurrentScene()
{
    if (m_ActiveScene)
    {
        Logger::InfoFormat("Scene unloading: %s", m_ActiveSceneName.c_str());

        m_ActiveScene->Uninit();
        m_ActiveScene.reset();
        m_ActiveSceneName = "";

        Logger::Info("Scene unloaded");
    }
}

//==============================================================================
// シーン切り替え処理（フレーム開始時に実行）
//==============================================================================
void SceneManager::ProcessSceneChange()
{
    if (!m_SceneChangeRequested)
        return;

    m_SceneChangeRequested = false;

    // 現在のシーンをアンロード
    UnloadCurrentScene();

    // 新しいシーンを生成
    auto it = m_SceneFactories.find(m_NextSceneName);
    if (it != m_SceneFactories.end())
    {
        m_ActiveScene = it->second();
        m_ActiveSceneName = m_NextSceneName;

        Logger::InfoFormat("Scene loaded: %s", m_ActiveSceneName.c_str());

        // 初期化
        m_ActiveScene->Init();
    }

    m_NextSceneName = "";
}
