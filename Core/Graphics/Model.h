#pragma once

//==============================================================================
// Model.h - 3Dモデルクラス
//==============================================================================

#include "Core/System/main.h"
#include "Core/Math/Vector3.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//==============================================================================
// メッシュデータ
//==============================================================================
struct Mesh
{
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	UINT vertexCount = 0;
	UINT indexCount = 0;

	// マテリアル情報
	MATERIAL material;
	ID3D11ShaderResourceView* texture = nullptr;
};

//==============================================================================
// Model クラス
// - Assimpを使ったFBX/OBJモデルの読み込み
// - 複数メッシュ対応
//==============================================================================
class Model
{
public:
	Model();
	~Model();

	// コピー禁止
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	// モデル読み込み
	bool Load(const std::string& fileName);

	// 解放
	void Unload();

	// 描画
	void Draw();

	// メッシュ数を取得
	size_t GetMeshCount() const { return m_Meshes.size(); }

	// メッシュを取得
	Mesh* GetMesh(size_t index);

private:
	std::vector<Mesh> m_Meshes;
	std::string m_Directory;  // モデルファイルのディレクトリ

	// ノード処理（再帰）
	void ProcessNode(aiNode* node, const aiScene* scene);

	// メッシュ処理
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	// マテリアル処理
	void LoadMaterial(Mesh& mesh, aiMaterial* material, const aiScene* scene);

	// テクスチャパスを取得
	std::string GetTexturePath(aiMaterial* material, aiTextureType type);

	// 埋め込みテクスチャ処理
	ID3D11ShaderResourceView* LoadEmbeddedTexture(const aiTexture* texture);

	// 埋め込みテクスチャのキャッシュ
	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_EmbeddedTextures;
};
