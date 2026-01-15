//==============================================================================
// Model.cpp - 3Dモデルクラス実装
//==============================================================================

#include "Model.h"
#include "Renderer.h"
#include "Texture.h"
#include "Core/System/Logger.h"
#include <DirectXTex.h>

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
Model::Model()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
Model::~Model()
{
	Unload();
}

//==============================================================================
// モデル読み込み
//==============================================================================
bool Model::Load(const std::string& fileName)
{
	// 既存データを解放
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

	// Assimpでモデルを読み込み
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName,
		aiProcess_Triangulate |           // 三角形化
		aiProcess_FlipUVs |               // UV座標を反転
		aiProcess_CalcTangentSpace |      // 接線計算
		aiProcess_GenNormals |            // 法線生成
		aiProcess_JoinIdenticalVertices | // 重複頂点を結合
		aiProcess_ConvertToLeftHanded     // 左手座標系に変換
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		Logger::ErrorFormat("Assimp error: %s", importer.GetErrorString());
		return false;
	}

	// ルートノードから処理開始
	ProcessNode(scene->mRootNode, scene);

	Logger::InfoFormat("Model loaded: %s (%zu meshes)", fileName.c_str(), m_Meshes.size());

	return true;
}

//==============================================================================
// 解放
//==============================================================================
void Model::Unload()
{
	for (auto& mesh : m_Meshes)
	{
		SAFE_RELEASE(mesh.vertexBuffer);
		SAFE_RELEASE(mesh.indexBuffer);
		// テクスチャはTexture::Loadで管理されているので解放しない
	}
	m_Meshes.clear();

	// 埋め込みテクスチャを解放
	for (auto& pair : m_EmbeddedTextures)
	{
		SAFE_RELEASE(pair.second);
	}
	m_EmbeddedTextures.clear();

	m_Directory = "";
}

//==============================================================================
// 描画
//==============================================================================
void Model::Draw()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	for (auto& mesh : m_Meshes)
	{
		// マテリアル設定
		Renderer::SetMaterial(mesh.material);

		// テクスチャ設定
		if (mesh.texture)
		{
			Texture::Set(mesh.texture, 0);
		}

		// 頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &mesh.vertexBuffer, &stride, &offset);

		// インデックスバッファ設定
		context->IASetIndexBuffer(mesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// トポロジー設定
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 描画
		context->DrawIndexed(mesh.indexCount, 0, 0);
	}
}

//==============================================================================
// メッシュを取得
//==============================================================================
Mesh* Model::GetMesh(size_t index)
{
	if (index < m_Meshes.size())
	{
		return &m_Meshes[index];
	}
	return nullptr;
}

//==============================================================================
// ノード処理（再帰）
//==============================================================================
void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	// このノードの全メッシュを処理
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(ProcessMesh(mesh, scene));
	}

	// 子ノードを再帰処理
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

//==============================================================================
// メッシュ処理
//==============================================================================
Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	Mesh result;

	// 頂点データを作成
	std::vector<VERTEX_3D> vertices;
	vertices.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VERTEX_3D vertex;

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

		vertices.push_back(vertex);
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
	vbDesc.ByteWidth = sizeof(VERTEX_3D) * result.vertexCount;
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

	// マテリアル読み込み
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		LoadMaterial(result, material, scene);
	}
	else
	{
		// デフォルトマテリアル
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
// マテリアル処理
//==============================================================================
void Model::LoadMaterial(Mesh& mesh, aiMaterial* material, const aiScene* scene)
{
	aiColor4D color;

	// アンビエント
	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_AMBIENT, color))
	{
		mesh.material.Ambient = XMFLOAT4(color.r, color.g, color.b, color.a);
	}
	else
	{
		mesh.material.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	}

	// ディフューズ
	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color))
	{
		mesh.material.Diffuse = XMFLOAT4(color.r, color.g, color.b, color.a);
	}
	else
	{
		mesh.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// スペキュラー
	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color))
	{
		mesh.material.Specular = XMFLOAT4(color.r, color.g, color.b, color.a);
	}
	else
	{
		mesh.material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// エミッシブ
	if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, color))
	{
		mesh.material.Emission = XMFLOAT4(color.r, color.g, color.b, color.a);
	}
	else
	{
		mesh.material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// シャイニネス
	float shininess;
	if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, shininess))
	{
		mesh.material.Shininess = shininess;
	}
	else
	{
		mesh.material.Shininess = 0.0f;
	}

	// テクスチャ
	mesh.texture = nullptr;
	mesh.material.TextureEnable = FALSE;

	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		aiString path;
		if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, 0, &path))
		{
			std::string texturePath = path.C_Str();

			Logger::InfoFormat("Texture path in FBX: %s", texturePath.c_str());

			// 埋め込みテクスチャかチェック（GetEmbeddedTextureで確認）
			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());

			if (embeddedTexture)
			{
				// 埋め込みテクスチャ
				Logger::Info("Found embedded texture");

				// キャッシュを確認
				auto it = m_EmbeddedTextures.find(texturePath);
				if (it != m_EmbeddedTextures.end())
				{
					mesh.texture = it->second;
				}
				else
				{
					// 埋め込みテクスチャを読み込み
					mesh.texture = LoadEmbeddedTexture(embeddedTexture);
					if (mesh.texture)
					{
						m_EmbeddedTextures[texturePath] = mesh.texture;
					}
				}
			}
			else if (!texturePath.empty() && texturePath[0] == '*')
			{
				// *で始まる埋め込みテクスチャ参照
				int textureIndex = std::atoi(texturePath.c_str() + 1);

				if (textureIndex >= 0 && textureIndex < (int)scene->mNumTextures)
				{
					auto it = m_EmbeddedTextures.find(texturePath);
					if (it != m_EmbeddedTextures.end())
					{
						mesh.texture = it->second;
					}
					else
					{
						mesh.texture = LoadEmbeddedTexture(scene->mTextures[textureIndex]);
						if (mesh.texture)
						{
							m_EmbeddedTextures[texturePath] = mesh.texture;
						}
					}
				}
			}
			else
			{
				// 外部テクスチャファイル

				// ファイル名だけを取り出す
				std::string fileName = texturePath;

				// バックスラッシュをスラッシュに統一
				for (char& c : fileName)
				{
					if (c == '\\') c = '/';
				}

				// 最後のスラッシュ以降を取得（ファイル名のみ）
				size_t lastSlash = fileName.find_last_of('/');
				if (lastSlash != std::string::npos)
				{
					fileName = fileName.substr(lastSlash + 1);
				}

				// モデルと同じディレクトリで探す
				std::string fullPath = m_Directory + fileName;
				mesh.texture = Texture::Load(fullPath);

				// 見つからなければ他の場所を試す
				if (!mesh.texture)
				{
					fullPath = "Asset/Texture/" + fileName;
					mesh.texture = Texture::Load(fullPath);
				}

				if (!mesh.texture)
				{
					fullPath = m_Directory + "Textures/" + fileName;
					mesh.texture = Texture::Load(fullPath);
				}

				if (!mesh.texture)
				{
					Logger::WarningFormat("Texture not found: %s", fileName.c_str());
				}
			}

			mesh.material.TextureEnable = (mesh.texture != nullptr) ? TRUE : FALSE;
		}
	}
}
//==============================================================================
// テクスチャパスを取得
//==============================================================================
std::string Model::GetTexturePath(aiMaterial* material, aiTextureType type)
{
	if (material->GetTextureCount(type) > 0)
	{
		aiString path;
		if (AI_SUCCESS == material->GetTexture(type, 0, &path))
		{
			std::string texturePath = path.C_Str();

			// 相対パスならディレクトリを結合
			if (texturePath.find(':') == std::string::npos)
			{
				texturePath = m_Directory + texturePath;
			}

			return texturePath;
		}
	}
	return "";
}

//==============================================================================
// 埋め込みテクスチャを読み込み
//==============================================================================
ID3D11ShaderResourceView* Model::LoadEmbeddedTexture(const aiTexture* texture)
{
	ID3D11ShaderResourceView* srv = nullptr;

	if (texture->mHeight == 0)
	{
		// 圧縮テクスチャ（PNG, JPGなど）
		DirectX::ScratchImage scratchImage;
		HRESULT hr = E_FAIL;

		// データをuint8_t*にキャスト
		const uint8_t* data = reinterpret_cast<const uint8_t*>(texture->pcData);
		size_t size = static_cast<size_t>(texture->mWidth);

		// WICで読み込み（PNG, JPG, BMPなど）
		hr = DirectX::LoadFromWICMemory(
			data,
			size,
			DirectX::WIC_FLAGS_NONE,
			nullptr,
			scratchImage
		);

		// WICで失敗したらDDSとして試す
		if (FAILED(hr))
		{
			hr = DirectX::LoadFromDDSMemory(
				data,
				size,
				DirectX::DDS_FLAGS_NONE,
				nullptr,
				scratchImage
			);
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

			if (SUCCEEDED(hr))
			{
				Logger::Info("Embedded texture loaded");
			}
		}
		else
		{
			Logger::Error("Failed to load embedded texture");
		}
	}
	else
	{
		// 非圧縮テクスチャ（ARGB8888）
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

			if (SUCCEEDED(hr))
			{
				Logger::Info("Embedded texture loaded (uncompressed)");
			}
		}
	}

	return srv;
}
