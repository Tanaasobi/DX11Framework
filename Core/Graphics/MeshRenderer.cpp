//==============================================================================
// MeshRenderer.cpp - メッシュ描画コンポーネント実装
//==============================================================================

#include "MeshRenderer.h"
#include "Renderer.h"
#include "../System/Logger.h"
#include "../Object/GameObject.h"
#include "../Object/Transform.h"

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
MeshRenderer::MeshRenderer()
{
	// デフォルトマテリアル
	m_Material.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_Material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_Material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_Material.Shininess = 0.0f;
	m_Material.TextureEnable = FALSE;

	for (int i = 0; i < 8; i++)
	{
		m_Textures[i] = nullptr;
	}
}

//==============================================================================
// デストラクタ
//==============================================================================
MeshRenderer::~MeshRenderer()
{
	ReleaseBuffers();

	if (m_OwnsShader && m_Shader)
	{
		delete m_Shader;
		m_Shader = nullptr;
	}
}

//==============================================================================
// バッファ解放
//==============================================================================
void MeshRenderer::ReleaseBuffers()
{
	if (m_OwnsVertexBuffer && m_VertexBuffer)
	{
		m_VertexBuffer->Release();
	}
	m_VertexBuffer = nullptr;
	m_OwnsVertexBuffer = false;

	if (m_OwnsIndexBuffer && m_IndexBuffer)
	{
		m_IndexBuffer->Release();
	}
	m_IndexBuffer = nullptr;
	m_OwnsIndexBuffer = false;
}

//==============================================================================
// 描画
//==============================================================================
void MeshRenderer::Render()
{
	if (!m_Shader || !m_VertexBuffer) return;

	// シェーダーをセット
	m_Shader->Set();

	// ワールド行列をセット
	Transform* transform = GetGameObject()->GetTransform();
	Renderer::SetWorldMatrix(transform->GetWorldMatrix());

	// マテリアルをセット
	Renderer::SetMaterial(m_Material);

	// テクスチャをセット
	for (int i = 0; i < 8; i++)
	{
		if (m_Textures[i])
		{
			Texture::Set(m_Textures[i], i);
		}
	}

	// 頂点バッファをセット
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_Stride, &offset);

	// プリミティブトポロジーをセット
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(m_Topology);

	// 描画
	if (m_IndexBuffer)
	{
		Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		Renderer::GetDeviceContext()->DrawIndexed(m_IndexCount, 0, 0);
	}
	else
	{
		Renderer::GetDeviceContext()->Draw(m_VertexCount, 0);
	}
}

//==============================================================================
// 頂点バッファ設定
//==============================================================================
void MeshRenderer::SetVertexBuffer(ID3D11Buffer* vertexBuffer, UINT vertexCount, UINT stride)
{
	ReleaseBuffers();

	m_VertexBuffer = vertexBuffer;
	m_VertexCount = vertexCount;
	m_Stride = stride;
	m_OwnsVertexBuffer = false;
}

//==============================================================================
// インデックスバッファ設定
//==============================================================================
void MeshRenderer::SetIndexBuffer(ID3D11Buffer* indexBuffer, UINT indexCount)
{
	if (m_OwnsIndexBuffer && m_IndexBuffer)
	{
		m_IndexBuffer->Release();
	}

	m_IndexBuffer = indexBuffer;
	m_IndexCount = indexCount;
	m_OwnsIndexBuffer = false;
}

//==============================================================================
// プリミティブタイプ設定
//==============================================================================
void MeshRenderer::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_Topology = topology;
}

//==============================================================================
// シェーダー設定
//==============================================================================
void MeshRenderer::SetShader(IShader* shader)
{
	if (m_OwnsShader && m_Shader)
	{
		delete m_Shader;
	}

	m_Shader = shader;
	m_OwnsShader = false;
}

//==============================================================================
// テクスチャ設定
//==============================================================================
void MeshRenderer::SetTexture(ID3D11ShaderResourceView* texture, UINT slot)
{
	if (slot < 8)
	{
		m_Textures[slot] = texture;
		m_Material.TextureEnable = (texture != nullptr) ? TRUE : FALSE;
	}
}

void MeshRenderer::SetTexture(const std::string& fileName, UINT slot)
{
	SetTexture(Texture::Load(fileName), slot);
}

//==============================================================================
// マテリアル設定
//==============================================================================
void MeshRenderer::SetMaterial(const MATERIAL& material)
{
	m_Material = material;
}

//==============================================================================
// 三角形作成
//==============================================================================
void MeshRenderer::CreateTriangle()
{
	ReleaseBuffers();

	VERTEX_3D vertices[] =
	{
		{ XMFLOAT3(0.0f,  0.5f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT4(1,0,0,1), XMFLOAT2(0.5f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT4(0,1,0,1), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT4(0,0,1,1), XMFLOAT2(0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_VertexBuffer);
	if (SUCCEEDED(hr))
	{
		m_VertexCount = 3;
		m_Stride = sizeof(VERTEX_3D);
		m_OwnsVertexBuffer = true;
	}
}

//==============================================================================
// 四角形作成
//==============================================================================
void MeshRenderer::CreateQuad(float width, float height)
{
	ReleaseBuffers();

	float hw = width * 0.5f;
	float hh = height * 0.5f;

	VERTEX_3D vertices[] =
	{
		{ XMFLOAT3(-hw,  hh, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(hw,  hh, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-hw, -hh, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(hw, -hh, 0.0f), XMFLOAT3(0,0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_VertexBuffer);
	if (FAILED(hr)) return;

	m_VertexCount = 4;
	m_Stride = sizeof(VERTEX_3D);
	m_OwnsVertexBuffer = true;

	// インデックスバッファ
	UINT indices[] = { 0, 1, 2, 2, 1, 3 };

	bufferDesc.ByteWidth = sizeof(indices);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices;

	hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_IndexBuffer);
	if (SUCCEEDED(hr))
	{
		m_IndexCount = 6;
		m_OwnsIndexBuffer = true;
	}
}

//==============================================================================
// 立方体作成
//==============================================================================
void MeshRenderer::CreateCube(float size)
{
	ReleaseBuffers();

	float s = size * 0.5f;

	VERTEX_3D vertices[] =
	{
		// 前面
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(0, 0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },
		{ XMFLOAT3(s,  s, -s), XMFLOAT3(0, 0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(0, 0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
		{ XMFLOAT3(s, -s, -s), XMFLOAT3(0, 0,-1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },

		// 背面
		{ XMFLOAT3(s,  s,  s), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },
		{ XMFLOAT3(-s,  s,  s), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
		{ XMFLOAT3(s, -s,  s), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(0, 0, 1), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },

		// 上面
		{ XMFLOAT3(-s,  s,  s), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },
		{ XMFLOAT3(s,  s,  s), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
		{ XMFLOAT3(s,  s, -s), XMFLOAT3(0, 1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },

		// 下面
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(0,-1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },
		{ XMFLOAT3(s, -s, -s), XMFLOAT3(0,-1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(0,-1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
		{ XMFLOAT3(s, -s,  s), XMFLOAT3(0,-1, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },

		// 右面
		{ XMFLOAT3(s,  s, -s), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },
		{ XMFLOAT3(s,  s,  s), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
		{ XMFLOAT3(s, -s, -s), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
		{ XMFLOAT3(s, -s,  s), XMFLOAT3(1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },

		// 左面
		{ XMFLOAT3(-s,  s,  s), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,0) },
		{ XMFLOAT3(-s,  s, -s), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,0) },
		{ XMFLOAT3(-s, -s,  s), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(0,1) },
		{ XMFLOAT3(-s, -s, -s), XMFLOAT3(-1, 0, 0), XMFLOAT4(1,1,1,1), XMFLOAT2(1,1) },
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_VertexBuffer);
	if (FAILED(hr)) return;

	m_VertexCount = 24;
	m_Stride = sizeof(VERTEX_3D);
	m_OwnsVertexBuffer = true;

	// インデックスバッファ
	UINT indices[] =
	{
		0,  1,  2,  2,  1,  3,   // 前
		4,  5,  6,  6,  5,  7,   // 後
		8,  9, 10, 10,  9, 11,   // 上
	   12, 13, 14, 14, 13, 15,   // 下
	   16, 17, 18, 18, 17, 19,   // 右
	   20, 21, 22, 22, 21, 23,   // 左
	};

	bufferDesc.ByteWidth = sizeof(indices);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = indices;

	hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_IndexBuffer);
	if (SUCCEEDED(hr))
	{
		m_IndexCount = 36;
		m_OwnsIndexBuffer = true;
	}
}
