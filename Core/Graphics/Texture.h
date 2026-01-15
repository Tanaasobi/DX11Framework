#pragma once

//==============================================================================
// Texture.h - テクスチャ管理クラス
//==============================================================================

#include "../System/main.h"

//==============================================================================
// Texture クラス
// - テクスチャの読み込みとフライウェイト管理
// - 同一ファイルの多重ロードを防ぐ
//==============================================================================
class Texture
{
public:
    // 初期化・終了
    static void Init();
    static void Uninit();

    // テクスチャ読み込み（既に読み込み済みならキャッシュから返す）
    static ID3D11ShaderResourceView* Load(const std::string& fileName);
    static ID3D11ShaderResourceView* Load(const std::wstring& fileName);

    // 特定のテクスチャを解放
    static void Unload(const std::string& fileName);

    // 全テクスチャを解放
    static void UnloadAll();

    // テクスチャをシェーダーにセット
    static void Set(ID3D11ShaderResourceView* texture, UINT slot = 0);

private:
    static std::unordered_map<std::string, ID3D11ShaderResourceView*> m_TexturePool;

    // ワイド文字列をマルチバイトに変換
    static std::string WideToMultiByte(const std::wstring& wstr);

    // マルチバイトをワイド文字列に変換
    static std::wstring MultiByteToWide(const std::string& str);
};
