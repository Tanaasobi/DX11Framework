#pragma once

//==============================================================================
// MeshRenderer.h - メッシュ描画コンポーネント
//==============================================================================

#include "Core/Object/Component.h"
#include "Core/System/main.h"
#include "Shader/IShader.h"
#include "Texture.h"

//==============================================================================
// MeshRenderer クラス
//==============================================================================
class MeshRenderer : public Component
{
public:
	MeshRenderer();
	virtual ~MeshRenderer();

	void Render() override;

	// バッファ設定
	void SetVertexBuffer(ID3D11Buffer* vertexBuffer, UINT vertexCount, UINT stride);
	void SetIndexBuffer(ID3D11Buffer* indexBuffer, UINT indexCount);
	void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

	// シェーダー設定
	void SetShader(IShader* shader);

	// テクスチャ設定
	void SetTexture(ID3D11ShaderResourceView* texture, UINT slot = 0);
	void SetTexture(const std::string& fileName, UINT slot = 0);

	// マテリアル設定
	void SetMaterial(const MATERIAL& material);

	// 簡易メッシュ作成
	void CreateTriangle();
	void CreateQuad(float width = 1.0f, float height = 1.0f);
	void CreateCube(float size = 1.0f);

private:
	void ReleaseBuffers();

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;
	UINT m_VertexCount = 0;
	UINT m_IndexCount = 0;
	UINT m_Stride = 0;
	D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	bool m_OwnsVertexBuffer = false;
	bool m_OwnsIndexBuffer = false;
	bool m_OwnsShader = false;

	IShader* m_Shader = nullptr;  // Shader* から IShader* に変更
	MATERIAL m_Material;
	ID3D11ShaderResourceView* m_Textures[8];
};
