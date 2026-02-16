//==============================================================================
// ParticleRenderer.cpp - GPUインスタンシング描画実装
//==============================================================================

#include "ParticleRenderer.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"

using namespace DirectX;

//==============================================================================
// ビルボード頂点
//==============================================================================
struct ParticleVertex
{
	XMFLOAT3 Position;
	XMFLOAT2 TexCoord;
};

//==============================================================================
// コンストラクタ
//==============================================================================
ParticleRenderer::ParticleRenderer()
{
}

//==============================================================================
// デストラクタ
//==============================================================================
ParticleRenderer::~ParticleRenderer()
{
	Uninit();
}

//==============================================================================
// 初期化
//==============================================================================
bool ParticleRenderer::Init(int maxParticles)
{
	m_MaxParticles = maxParticles;

	if (!CreateVertexBuffer()) return false;
	if (!CreateInstanceBuffer(maxParticles)) return false;

	Logger::InfoFormat("ParticleRenderer initialized: max=%d", maxParticles);
	return true;
}

//==============================================================================
// 終了
//==============================================================================
void ParticleRenderer::Uninit()
{
	SAFE_RELEASE(m_InstanceBuffer);
	SAFE_RELEASE(m_VertexBuffer);
}

//==============================================================================
// 頂点バッファ作成（ビルボード四角形）
//==============================================================================
bool ParticleRenderer::CreateVertexBuffer()
{
	// 中心が原点の四角形（-0.5 ~ 0.5）
	ParticleVertex vertices[] =
	{
		{ XMFLOAT3(-0.5f,  0.5f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.5f,  0.5f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_VertexBuffer);
	return SUCCEEDED(hr);
}

//==============================================================================
// インスタンスバッファ作成
//==============================================================================
bool ParticleRenderer::CreateInstanceBuffer(int maxParticles)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(ParticleInstance) * maxParticles;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_InstanceBuffer);
	return SUCCEEDED(hr);
}

//==============================================================================
// インスタンスバッファ更新
//==============================================================================
void ParticleRenderer::UpdateInstanceBuffer(const std::vector<Particle>& particles)
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = context->Map(m_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr)) return;

	ParticleInstance* instances = static_cast<ParticleInstance*>(mapped.pData);

	int index = 0;
	for (const auto& p : particles)
	{
		if (!p.active) continue;
		if (index >= m_MaxParticles) break;

		instances[index].Position = p.position.ToXMFLOAT3();
		instances[index].Size = p.size;
		instances[index].Color = p.color;
		instances[index].Rotation = p.rotation;
		instances[index].Velocity = p.velocity.ToXMFLOAT3();
		index++;
	}

	context->Unmap(m_InstanceBuffer, 0);
}

//==============================================================================
// 描画
//==============================================================================
void ParticleRenderer::Render(int activeCount)
{
	if (activeCount == 0 || !m_Shader) return;

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	m_Shader->Set();

	if (m_Texture)
	{
		context->PSSetShaderResources(0, 1, &m_Texture);
	}

	if (m_AdditiveBlend)
	{
		Renderer::SetBlendMode(BlendMode::Add);
	}
	else
	{
		Renderer::SetBlendMode(BlendMode::Alpha);
	}

	// 深度テスト有効、書き込み無効（オブジェクトの後ろに隠れる）
	Renderer::SetDepthEnable(true);
	Renderer::SetDepthWriteEnable(false);
	Renderer::SetCullingMode(false); // カリング無効（両面描画）

	ID3D11Buffer* buffers[2] = { m_VertexBuffer, m_InstanceBuffer };
	UINT strides[2] = { sizeof(ParticleVertex), sizeof(ParticleInstance) };
	UINT offsets[2] = { 0, 0 };
	context->IASetVertexBuffers(0, 2, buffers, strides, offsets);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->DrawInstanced(4, activeCount, 0, 0);

	// 設定を戻す
	Renderer::SetCullingMode(true);
	Renderer::SetDepthWriteEnable(true);
	Renderer::SetBlendMode(BlendMode::Alpha);
}
