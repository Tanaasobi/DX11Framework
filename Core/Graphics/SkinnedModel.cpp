//==============================================================================
// SkinnedModel.cpp - スキンメッシュモデル実装
//==============================================================================

#include "SkinnedModel.h"
#include "Renderer.h"
#include "Texture.h"
#include "Core/System/Logger.h"
#include <DirectXTex.h>

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
SkinnedModel::SkinnedModel()
{
	m_Skeleton = std::make_shared<Skeleton>();
}

//==============================================================================
// デストラクタ
//==============================================================================
SkinnedModel::~SkinnedModel()
{
	Unload();
}

//==============================================================================
// モデル読み込み
//==============================================================================
bool SkinnedModel::Load(const std::string& fileName)
{
	Unload();

	// ディレクトリを取得
	size_t lastSlash = fileName.find_last_of("/\\");
	if (lastSlash != std::string::npos)
	{
		m_Directory = fileName.substr(0, lastSlash + 1);
	}
	else
	{
		m_Directory = "";
	}

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace |
		aiProcess_GenNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ConvertToLeftHanded |
		aiProcess_LimitBoneWeights  // ボーン影響を4つに制限
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		Logger::ErrorFormat("Assimp error: %s", importer.GetErrorString());
		return false;
	}

	// スケルトンを構築
	BuildSkeleton(scene);

	// メッシュを処理
	ProcessNode(scene->mRootNode, scene);

	Logger::InfoFormat("SkinnedModel loaded: %s (%zu meshes, %zu bones)",
		fileName.c_str(), m_Meshes.size(), m_Skeleton->GetBoneCount());

	return true;
}

//==============================================================================
// 解放
//==============================================================================
void SkinnedModel::Unload()
{
	for (auto& mesh : m_Meshes)
	{
		SAFE_RELEASE(mesh.vertexBuffer);
		SAFE_RELEASE(mesh.indexBuffer);
	}
	m_Meshes.clear();

	for (auto& pair : m_EmbeddedTextures)
	{
		SAFE_RELEASE(pair.second);
	}
	m_EmbeddedTextures.clear();

	m_Skeleton = std::make_shared<Skeleton>();
	m_Directory = "";
}

//==============================================================================
// 描画
//==============================================================================
void SkinnedModel::Draw()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	for (auto& mesh : m_Meshes)
	{
		Renderer::SetMaterial(mesh.material);

		if (mesh.texture)
		{
			Texture::Set(mesh.texture, 0);
		}

		UINT stride = sizeof(VERTEX_3D_SKINNED);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);
		context->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->DrawIndexed(mesh.indexCount, 0, 0);
	}
}

//==============================================================================
// スケルトン構築
//==============================================================================
void SkinnedModel::BuildSkeleton(const aiScene* scene)
{
	// 全ボーン名を収集
	std::unordered_set<std::string> boneNames;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumBones; j++)
		{
			boneNames.insert(mesh->mBones[j]->mName.C_Str());
		}
	}

	// ノード階層を辿ってボーンを登録
	ProcessBoneNode(scene->mRootNode, -1);

	// オフセット行列を設定
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumBones; j++)
		{
			aiBone* bone = mesh->mBones[j];
			std::string boneName = bone->mName.C_Str();

			Bone* skeletonBone = m_Skeleton->GetBone(boneName);
			if (skeletonBone)
			{
				aiMatrix4x4 m = bone->mOffsetMatrix;
				skeletonBone->offsetMatrix = XMMATRIX(
					m.a1, m.b1, m.c1, m.d1,
					m.a2, m.b2, m.c2, m.d2,
					m.a3, m.b3, m.c3, m.d3,
					m.a4, m.b4, m.c4, m.d4
				);
			}
		}
	}
}

//==============================================================================
// ボーンノード処理（再帰）
//==============================================================================
void SkinnedModel::ProcessBoneNode(aiNode* node, int parentIndex)
{
	std::string nodeName = node->mName.C_Str();

	// ボーンとして追加
	int boneIndex = m_Skeleton->AddBone(nodeName, parentIndex);

	// 初期ローカル変換を設定
	Bone* bone = m_Skeleton->GetBone(boneIndex);
	if (bone)
	{
		aiMatrix4x4 m = node->mTransformation;

		// 行列からTRS分解
		aiVector3D scaling, position;
		aiQuaternion rotation;
		m.Decompose(scaling, rotation, position);

		bone->localPosition = Vector3(position.x, position.y, position.z);
		bone->localRotation = Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);
		bone->localScale = Vector3(scaling.x, scaling.y, scaling.z);
	}

	// 子ノードを処理
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessBoneNode(node->mChildren[i], boneIndex);
	}
}

//==============================================================================
// ノード処理
//==============================================================================
void SkinnedModel::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		if (mesh->HasBones())
		{
			m_Meshes.push_back(ProcessMesh(mesh, scene));
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

//==============================================================================
// メッシュ処理
//==============================================================================
SkinnedMesh SkinnedModel::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	SkinnedMesh result;

	// 頂点データを作成
	std::vector<VERTEX_3D_SKINNED> vertices;
	vertices.resize(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VERTEX_3D_SKINNED& vertex = vertices[i];

		// 位置
		vertex.Position.x = mesh->mVertices[i].x;
		vertex.Position.y = mesh->mVertices[i].y;
		vertex.Position.z = mesh->mVertices[i].z;

		// 法線
		if (mesh->HasNormals())
		{
			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;
		}
		else
		{
			vertex.Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		}

		// テクスチャ座標
		if (mesh->mTextureCoords[0])
		{
			vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
			vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vertex.TexCoord = XMFLOAT2(0.0f, 0.0f);
		}

		// 頂点カラー
		if (mesh->mColors[0])
		{
			vertex.Diffuse.x = mesh->mColors[0][i].r;
			vertex.Diffuse.y = mesh->mColors[0][i].g;
			vertex.Diffuse.z = mesh->mColors[0][i].b;
			vertex.Diffuse.w = mesh->mColors[0][i].a;
		}
		else
		{
			vertex.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		// ボーン情報を初期化
		for (int j = 0; j < 4; j++)
		{
			vertex.BoneIndices[j] = 0;
			vertex.BoneWeights[j] = 0.0f;
		}
	}

	// ボーンウェイトを設定
	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		aiBone* bone = mesh->mBones[i];
		std::string boneName = bone->mName.C_Str();

		int boneIndex = m_Skeleton->GetBoneIndex(boneName);
		if (boneIndex < 0) continue;

		for (unsigned int j = 0; j < bone->mNumWeights; j++)
		{
			unsigned int vertexId = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;

			VERTEX_3D_SKINNED& vertex = vertices[vertexId];

			// 空いているスロットを探す
			for (int k = 0; k < 4; k++)
			{
				if (vertex.BoneWeights[k] == 0.0f)
				{
					vertex.BoneIndices[k] = static_cast<UINT>(boneIndex);
					vertex.BoneWeights[k] = weight;
					break;
				}
			}
		}
	}

	// ウェイトを正規化
	for (auto& vertex : vertices)
	{
		float totalWeight = vertex.BoneWeights[0] + vertex.BoneWeights[1] +
			vertex.BoneWeights[2] + vertex.BoneWeights[3];

		if (totalWeight > 0.0f)
		{
			for (int i = 0; i < 4; i++)
			{
				vertex.BoneWeights[i] /= totalWeight;
			}
		}
		else
		{
			// ウェイトがない場合はルートボーンに100%
			vertex.BoneIndices[0] = 0;
			vertex.BoneWeights[0] = 1.0f;
		}
	}

	// インデックスデータを作成
	std::vector<UINT> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	result.vertexCount = static_cast<UINT>(vertices.size());
	result.indexCount = static_cast<UINT>(indices.size());

	// 頂点バッファ作成
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(VERTEX_3D_SKINNED) * result.vertexCount;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices.data();

	Renderer::GetDevice()->CreateBuffer(&vbDesc, &vbData, &result.vertexBuffer);

	// インデックスバッファ作成
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(UINT) * result.indexCount;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices.data();

	Renderer::GetDevice()->CreateBuffer(&ibDesc, &ibData, &result.indexBuffer);

	// マテリアル
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		LoadMaterial(result, material, scene);
	}
	else
	{
		result.material.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		result.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		result.material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		result.material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		result.material.Shininess = 0.0f;
		result.material.TextureEnable = FALSE;
	}

	return result;
}

//==============================================================================
// マテリアル読み込み
//==============================================================================
void SkinnedModel::LoadMaterial(SkinnedMesh& mesh, aiMaterial* material, const aiScene* scene)
{
	aiColor4D color;

	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color))
		mesh.material.Ambient = XMFLOAT4(color.r, color.g, color.b, color.a);
	else
		mesh.material.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color))
		mesh.material.Diffuse = XMFLOAT4(color.r, color.g, color.b, color.a);
	else
		mesh.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color))
		mesh.material.Specular = XMFLOAT4(color.r, color.g, color.b, color.a);
	else
		mesh.material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, color))
		mesh.material.Emission = XMFLOAT4(color.r, color.g, color.b, color.a);
	else
		mesh.material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	float shininess;
	if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess))
		mesh.material.Shininess = shininess;
	else
		mesh.material.Shininess = 0.0f;

	// テクスチャ
	mesh.texture = nullptr;
	mesh.material.TextureEnable = FALSE;

	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString path;
		if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, 0, &path))
		{
			std::string texturePath = path.C_Str();

			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());

			if (embeddedTexture)
			{
				auto it = m_EmbeddedTextures.find(texturePath);
				if (it != m_EmbeddedTextures.end())
				{
					mesh.texture = it->second;
				}
				else
				{
					mesh.texture = LoadEmbeddedTexture(embeddedTexture);
					if (mesh.texture)
					{
						m_EmbeddedTextures[texturePath] = mesh.texture;
					}
				}
			}
			else
			{
				std::string fileName = texturePath;
				for (char& c : fileName)
				{
					if (c == '\\') c = '/';
				}

				size_t lastSlash = fileName.find_last_of('/');
				if (lastSlash != std::string::npos)
				{
					fileName = fileName.substr(lastSlash + 1);
				}

				std::string fullPath = m_Directory + fileName;
				mesh.texture = Texture::Load(fullPath);

				if (!mesh.texture)
				{
					fullPath = "Asset/Texture/" + fileName;
					mesh.texture = Texture::Load(fullPath);
				}
			}

			mesh.material.TextureEnable = (mesh.texture != nullptr) ? TRUE : FALSE;
		}
	}
}

//==============================================================================
// 埋め込みテクスチャ読み込み
//==============================================================================
ID3D11ShaderResourceView* SkinnedModel::LoadEmbeddedTexture(const aiTexture* texture)
{
	ID3D11ShaderResourceView* srv = nullptr;

	if (texture->mHeight == 0)
	{
		DirectX::ScratchImage scratchImage;
		HRESULT hr = E_FAIL;

		const uint8_t* data = reinterpret_cast<const uint8_t*>(texture->pcData);
		size_t size = static_cast<size_t>(texture->mWidth);

		hr = DirectX::LoadFromWICMemory(data, size, DirectX::WIC_FLAGS_NONE, nullptr, scratchImage);

		if (FAILED(hr))
		{
			hr = DirectX::LoadFromDDSMemory(data, size, DirectX::DDS_FLAGS_NONE, nullptr, scratchImage);
		}

		if (SUCCEEDED(hr))
		{
			hr = DirectX::CreateShaderResourceView(
				Renderer::GetDevice(),
				scratchImage.GetImages(),
				scratchImage.GetImageCount(),
				scratchImage.GetMetadata(),
				&srv
			);
		}
	}
	else
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = texture->mWidth;
		desc.Height = texture->mHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = texture->pcData;
		initData.SysMemPitch = texture->mWidth * 4;

		ID3D11Texture2D* texture2D = nullptr;
		HRESULT hr = Renderer::GetDevice()->CreateTexture2D(&desc, &initData, &texture2D);

		if (SUCCEEDED(hr))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = desc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;

			hr = Renderer::GetDevice()->CreateShaderResourceView(texture2D, &srvDesc, &srv);
			texture2D->Release();
		}
	}

	return srv;
}
