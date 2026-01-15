#pragma once

//==============================================================================
// SkinnedModel.h - スキンメッシュモデル
//==============================================================================

#include "Core/System/main.h"
#include "Core/Animation/Skeleton.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>

//==============================================================================
// スキンメッシュデータ
//==============================================================================
struct SkinnedMesh
{
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	UINT vertexCount = 0;
	UINT indexCount = 0;

	MATERIAL material;
	ID3D11ShaderResourceView* texture = nullptr;
};

//==============================================================================
// SkinnedModel クラス
// - ボーン付きFBXモデルの読み込み
// - スケルトン情報を含む
//==============================================================================
class SkinnedModel
{
public:
	SkinnedModel();
	~SkinnedModel();

	// コピー禁止
	SkinnedModel(const SkinnedModel&) = delete;
	SkinnedModel& operator=(const SkinnedModel&) = delete;

	// モデル読み込み
	bool Load(const std::string& fileName);

	// 解放
	void Unload();

	// 描画（ボーン行列はAnimatorから取得する想定）
	void Draw();

	// スケルトン取得
	std::shared_ptr<Skeleton> GetSkeleton() const { return m_Skeleton; }

	// メッシュ数
	size_t GetMeshCount() const { return m_Meshes.size(); }

private:
	std::vector<SkinnedMesh> m_Meshes;
	std::shared_ptr<Skeleton> m_Skeleton;
	std::string m_Directory;

	// 埋め込みテクスチャキャッシュ
	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_EmbeddedTextures;

	// ノード処理
	void ProcessNode(aiNode* node, const aiScene* scene);

	// メッシュ処理
	SkinnedMesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	// スケルトン構築
	void BuildSkeleton(const aiScene* scene);
	void ProcessBoneNode(aiNode* node, int parentIndex);

	// マテリアル読み込み
	void LoadMaterial(SkinnedMesh& mesh, aiMaterial* material, const aiScene* scene);

	// 埋め込みテクスチャ読み込み
	ID3D11ShaderResourceView* LoadEmbeddedTexture(const aiTexture* texture);
};
