//==============================================================================
// Texture.cpp - テクスチャ管理クラス実装
//==============================================================================

#include "Texture.h"
#include "Renderer.h"
#include "../System/Logger.h"
#include <DirectXTex.h>

#pragma comment(lib, "DirectXTex.lib")

//==============================================================================
// 静的メンバ変数の定義
//==============================================================================
std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture::m_TexturePool;

//==============================================================================
// 初期化
//==============================================================================
void Texture::Init()
{
    // COM初期化（DirectXTexで必要）
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    Logger::Info("Texture system initialized");
}

//==============================================================================
// 終了
//==============================================================================
void Texture::Uninit()
{
    UnloadAll();
    CoUninitialize();

    Logger::Info("Texture system uninitialized");
}

//==============================================================================
// テクスチャ読み込み（string版）
//==============================================================================
ID3D11ShaderResourceView* Texture::Load(const std::string& fileName)
{
    // 既に読み込み済みならキャッシュから返す
    auto it = m_TexturePool.find(fileName);
    if (it != m_TexturePool.end())
    {
        return it->second;
    }

    // ワイド文字列に変換
    std::wstring wideFileName = MultiByteToWide(fileName);

    // 拡張子を取得
    std::wstring ext = wideFileName.substr(wideFileName.find_last_of(L".") + 1);

    // 画像を読み込み
    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratchImage;
    HRESULT hr;

    if (ext == L"tga" || ext == L"TGA")
    {
        hr = DirectX::LoadFromTGAFile(wideFileName.c_str(), &metadata, scratchImage);
    }
    else if (ext == L"dds" || ext == L"DDS")
    {
        hr = DirectX::LoadFromDDSFile(wideFileName.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage);
    }
    else
    {
        // PNG, JPG, BMP など（WIC経由）
        hr = DirectX::LoadFromWICFile(wideFileName.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);
    }

    if (FAILED(hr))
    {
        Logger::ErrorFormat("Failed to load texture: %s", fileName.c_str());
        return nullptr;
    }

    // シェーダーリソースビューを作成
    ID3D11ShaderResourceView* srv = nullptr;
    hr = DirectX::CreateShaderResourceView(
        Renderer::GetDevice(),
        scratchImage.GetImages(),
        scratchImage.GetImageCount(),
        metadata,
        &srv
    );

    if (FAILED(hr))
    {
        Logger::ErrorFormat("Failed to create SRV: %s", fileName.c_str());
        return nullptr;
    }

    // キャッシュに登録
    m_TexturePool[fileName] = srv;

    Logger::InfoFormat("Texture loaded: %s", fileName.c_str());

    return srv;
}

//==============================================================================
// テクスチャ読み込み（wstring版）
//==============================================================================
ID3D11ShaderResourceView* Texture::Load(const std::wstring& fileName)
{
    return Load(WideToMultiByte(fileName));
}

//==============================================================================
// 特定のテクスチャを解放
//==============================================================================
void Texture::Unload(const std::string& fileName)
{
    auto it = m_TexturePool.find(fileName);
    if (it != m_TexturePool.end())
    {
        if (it->second)
        {
            it->second->Release();
        }
        m_TexturePool.erase(it);

        Logger::InfoFormat("Texture unloaded: %s", fileName.c_str());
    }
}

//==============================================================================
// 全テクスチャを解放
//==============================================================================
void Texture::UnloadAll()
{
    for (auto& pair : m_TexturePool)
    {
        if (pair.second)
        {
            pair.second->Release();
        }
    }
    m_TexturePool.clear();

    Logger::Info("All textures unloaded");
}

//==============================================================================
// テクスチャをシェーダーにセット
//==============================================================================
void Texture::Set(ID3D11ShaderResourceView* texture, UINT slot)
{
    Renderer::GetDeviceContext()->PSSetShaderResources(slot, 1, &texture);
}

//==============================================================================
// ワイド文字列をマルチバイトに変換
//==============================================================================
std::string Texture::WideToMultiByte(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
    return str;
}

//==============================================================================
// マルチバイトをワイド文字列に変換
//==============================================================================
std::wstring Texture::MultiByteToWide(const std::string& str)
{
    if (str.empty()) return std::wstring();

    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}
