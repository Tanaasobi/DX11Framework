#include "TrailRenderer.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"
#include "Core/Object/Camera.h"
#include "Core/System/Time.h"
#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include "Core/Graphics/Shader/Shader.h"
#include <algorithm>

using namespace DirectX;

//==============================================================================
// 内部シェーダークラス (TrailShader)
//==============================================================================
class TrailShader : public Shader
{
public:
	bool CreateInputLayout(ID3DBlob* vsBlob) override
	{
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		HRESULT hr = Renderer::GetDevice()->CreateInputLayout(
			layout, ARRAYSIZE(layout),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			&m_InputLayout);
		return SUCCEEDED(hr);
	}
};

//==============================================================================
// TrailRenderer 実装
//==============================================================================
TrailRenderer::TrailRenderer()
{
}

TrailRenderer::~TrailRenderer()
{
	if (m_VertexBuffer) m_VertexBuffer->Release();
	if (m_Shader) delete m_Shader;
}

void TrailRenderer::Init(const std::string& texturePath, float width, float lifetime)
{
	m_Width = width;
	m_Lifetime = lifetime;

	// 滑らかに描画するために最小距離を小さく設定
	m_MinVertexDistance = 0.01f;

	m_Shader = new TrailShader();
	m_Shader->Load(L"Shader/TrailVertexShader.hlsl", L"Shader/TrailPixelShader.hlsl");

	m_TextureSRV = Texture::Load(texturePath);

	CreateVertexBuffer();
}

void TrailRenderer::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(TrailVertex) * 4000;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Renderer::GetDevice()->CreateBuffer(&bd, nullptr, &m_VertexBuffer);
}

void TrailRenderer::SetColor(const DirectX::XMFLOAT4& startColor, const DirectX::XMFLOAT4& endColor)
{
	m_StartColor = startColor;
	m_EndColor = endColor;
}

void TrailRenderer::Update(float deltaTime)
{
	if (!GetGameObject()) return;

	Vector3 posVec = GetGameObject()->GetTransform()->position;

	// 床(Y=0)との干渉を防ぐため、Y座標を少し浮かせて固定する
	// これにより「高さのブレ」によるメッシュの歪みを防ぎます
	float fixedY = 0.05f;

	XMFLOAT3 currentPos = { posVec.x, fixedY, posVec.z };
	float currentTime = Time::GetTotalTime();

	// ポイント追加判定
	bool shouldAdd = false;
	if (m_Points.empty())
	{
		shouldAdd = true;
	}
	else
	{
		XMFLOAT3 lastPos = m_Points.back().Position;
		float distSq = (currentPos.x - lastPos.x) * (currentPos.x - lastPos.x) +
			(currentPos.z - lastPos.z) * (currentPos.z - lastPos.z); // Y軸は無視してXZ距離で判定

		if (distSq > m_MinVertexDistance * m_MinVertexDistance)
		{
			shouldAdd = true;
		}
	}

	if (shouldAdd)
	{
		TrailPoint pt;
		pt.Position = currentPos;
		pt.Time = currentTime;
		m_Points.push_back(pt);
	}

	// 寿命削除
	while (!m_Points.empty())
	{
		if (currentTime - m_Points.front().Time > m_Lifetime)
		{
			m_Points.pop_front();
		}
		else
		{
			break;
		}
	}
}

void TrailRenderer::UpdateVertexBuffer()
{
	if (m_Points.size() < 2 || !m_VertexBuffer) return;

	std::vector<TrailVertex> vertices;
	vertices.reserve(m_Points.size() * 2);

	float currentTime = Time::GetTotalTime();

	// 床と水平にするための「上ベクトル」を(0,1,0)に固定
	XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	for (size_t i = 0; i < m_Points.size(); ++i)
	{
		const auto& pt = m_Points[i];
		XMVECTOR pCurrent = XMLoadFloat3(&pt.Position);

		// 進行方向ベクトルの計算
		XMVECTOR forward;

		if (i == m_Points.size() - 1 && i > 0)
		{
			// 最新の点: 1つ前の点からのベクトルを使用
			XMVECTOR pPrev = XMLoadFloat3(&m_Points[i - 1].Position);
			forward = XMVectorSubtract(pCurrent, pPrev);
		}
		else if (i < m_Points.size() - 1)
		{
			// それ以外の点: 次の点へのベクトルを使用
			XMVECTOR pNext = XMLoadFloat3(&m_Points[i + 1].Position);
			forward = XMVectorSubtract(pNext, pCurrent);
		}
		else // ポイントが1つしかない等の例外
		{
			forward = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f); // デフォルトX軸
		}

		// 長さが0に近い場合の安全対策
		if (XMVectorGetX(XMVector3LengthSq(forward)) < 0.00001f)
		{
			forward = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f); // 仮の向き
		}
		else
		{
			forward = XMVector3Normalize(forward);
		}

		// 進行方向(Forward)と上(Y軸)の外積で「右(Right)」を求める
		// これにより、リボンは常にXZ平面と平行になります
		XMVECTOR right = XMVector3Cross(forward, upVector);
		right = XMVector3Normalize(right);

		// 色と太さ
		float age = currentTime - pt.Time;
		float alphaRatio = 1.0f - (age / m_Lifetime);
		alphaRatio = std::max(0.0f, std::min(1.0f, alphaRatio));

		XMFLOAT4 color;
		color.x = m_StartColor.x * alphaRatio + m_EndColor.x * (1 - alphaRatio);
		color.y = m_StartColor.y * alphaRatio + m_EndColor.y * (1 - alphaRatio);
		color.z = m_StartColor.z * alphaRatio + m_EndColor.z * (1 - alphaRatio);
		color.w = m_StartColor.w * alphaRatio + m_EndColor.w * (1 - alphaRatio);

		// 太さ調整 (先端ほど細くする)
		float widthScale = alphaRatio;

		// UV
		float u = static_cast<float>(i) / (m_Points.size() - 1);

		TrailVertex v1, v2;
		XMVECTOR vOffset = XMVectorScale(right, m_Width * 0.5f * widthScale);

		// 左右の頂点を生成
		XMStoreFloat3(&v1.Position, XMVectorAdd(pCurrent, vOffset));
		v1.Color = color;
		v1.TexCoord = { u, 0.0f };

		XMStoreFloat3(&v2.Position, XMVectorSubtract(pCurrent, vOffset));
		v2.Color = color;
		v2.TexCoord = { u, 1.0f };

		vertices.push_back(v2);
		vertices.push_back(v1);
	}

	D3D11_MAPPED_SUBRESOURCE ms;
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	if (SUCCEEDED(context->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
	{
		size_t dataSize = vertices.size() * sizeof(TrailVertex);
		if (dataSize > sizeof(TrailVertex) * 4000) dataSize = sizeof(TrailVertex) * 4000;
		memcpy(ms.pData, vertices.data(), dataSize);
		context->Unmap(m_VertexBuffer, 0);
	}
}

void TrailRenderer::Render()
{
	// ポイントが少なければ描画しない
	if (m_Points.size() < 2) return;

	// 頂点バッファ更新
	UpdateVertexBuffer();

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// 深度ステート (奥行き判定はするが、書き込みはしない)
	Renderer::SetDepthEnable(true);
	Renderer::SetDepthWriteEnable(false);


	// ラスタライザステート 
	// カリングなし(CULL_NONE) & ソリッド表示(FILL_SOLID) を強制的に適用
	static ID3D11RasterizerState* cullNoneState = nullptr;
	if (!cullNoneState)
	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID; 
		desc.CullMode = D3D11_CULL_NONE;  // 裏面も描画する (カリングなし)
		desc.FrontCounterClockwise = FALSE;
		desc.DepthClipEnable = TRUE;
		Renderer::GetDevice()->CreateRasterizerState(&desc, &cullNoneState);
	}
	context->RSSetState(cullNoneState);

	// シェーダーセット
	m_Shader->Set();

	// 行列セット
	Renderer::SetWorldMatrix(XMMatrixIdentity());
	Camera* camera = Camera::GetMain();
	if (camera)
	{
		Renderer::SetViewMatrix(camera->GetViewMatrix());
		Renderer::SetProjectionMatrix(camera->GetProjectionMatrix());
	}

	// テクスチャセット
	if (m_TextureSRV)
	{
		context->PSSetShaderResources(0, 1, &m_TextureSRV);
	}

	// 描画実行
	UINT stride = sizeof(TrailVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT drawCount = (UINT)m_Points.size() * 2;
	// 上限キャップ (バッファサイズに合わせて調整)
	if (drawCount > 4000) drawCount = 4000;

	context->Draw(drawCount, 0);

	// ステートを元に戻す
	Renderer::SetDepthWriteEnable(true);

	// ラスタライザをデフォルト(裏面カリング)に戻しておく
	Renderer::SetCullingMode(true);
}
