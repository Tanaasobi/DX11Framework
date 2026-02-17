#include "RingGauge.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"
#include "Core/Object/Camera.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Graphics/Shader/GaugeShader.h"
#include <algorithm>

using namespace DirectX;

struct GaugeVertex
{
	XMFLOAT3 Position;
	XMFLOAT2 TexCoord;
};

RingGauge::RingGauge()
{
	m_BufferData.Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_BufferData.Progress = 0.0f;
	m_BufferData.InnerRadius = 0.4f;
}

RingGauge::~RingGauge()
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_ConstantBuffer) m_ConstantBuffer->Release();
	if (m_Shader) delete m_Shader;
}

void RingGauge::Init(float radius, float innerRadiusRatio)
{
	m_Radius = radius;
	m_BufferData.InnerRadius = 0.5f * innerRadiusRatio;

	// シェーダーロード
	m_Shader = new GaugeShader();
	m_Shader->Load(L"Shader/GaugeVertexShader.hlsl", L"Shader/GaugePixelShader.hlsl");

	// 頂点バッファ生成
	float yVal = 0.1f;

	GaugeVertex vertices[] = {
		{ XMFLOAT3(-0.5f, yVal,  0.5f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, yVal,  0.5f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, yVal, -0.5f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, yVal, -0.5f), XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(GaugeVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Renderer::GetDevice()->CreateBuffer(&bd, nullptr, &m_VertexBuffer);

	// 頂点データ転送用の構造体を用意
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	//第2引数に &initData を渡してデータを転送する
	Renderer::GetDevice()->CreateBuffer(&bd, &initData, &m_VertexBuffer);

	// 定数バッファ生成
	bd.ByteWidth = sizeof(GaugeBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Renderer::GetDevice()->CreateBuffer(&bd, nullptr, &m_ConstantBuffer);
}

void RingGauge::Update(float deltaTime)
{
}

void RingGauge::SetProgress(float progress)
{
	m_BufferData.Progress = std::clamp(progress, 0.0f, 1.0f);
}

void RingGauge::SetColor(float r, float g, float b, float a)
{
	m_BufferData.Color = XMFLOAT4(r, g, b, a);
}

void RingGauge::Render()
{
	if (!m_IsVisible || !m_Shader) return;

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// 定数バッファ更新
	context->UpdateSubresource(m_ConstantBuffer, 0, nullptr, &m_BufferData, 0, 0);

	// シェーダー設定
	m_Shader->Set();

	// Rendererの機能を使ってステートを設定
	// アルファブレンド有効
	Renderer::SetBlendMode(BlendMode::Alpha);

	// 深度書き込み無効（背景に埋もれないように）
	Renderer::SetDepthWriteEnable(false);

	// カリングなし
	Renderer::SetCullingMode(false);


	// 行列計算
	Transform* transform = GetGameObject()->GetTransform();
	XMMATRIX world = XMMatrixScaling(m_Radius * 2.0f, 1.0f, m_Radius * 2.0f) *
		XMMatrixTranslation(transform->position.x, transform->position.y, transform->position.z);

	Renderer::SetWorldMatrix(world);
	Camera* camera = Camera::GetMain();
	if (camera)
	{
		Renderer::SetViewMatrix(camera->GetViewMatrix());
		Renderer::SetProjectionMatrix(camera->GetProjectionMatrix());
	}

	// 定数バッファセット
	context->PSSetConstantBuffers(2, 1, &m_ConstantBuffer);

	// 描画
	UINT stride = sizeof(GaugeVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->Draw(4, 0);

	// ステートを不透明描画に戻しておく
	Renderer::SetDepthWriteEnable(true);
	Renderer::SetCullingMode(true);
}
