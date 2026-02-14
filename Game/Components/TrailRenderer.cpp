#include "TrailRenderer.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"
#include "Core/Object/Camera.h"
#include "Core/System/Time.h"
#include "Core/Graphics/Texture.h"
#include "Core/Graphics/Renderer.h"
#include "Core/System/Logger.h"
#include "Core/Graphics/Shader/Shader.h" // Shaderクラスの定義用

using namespace DirectX;

//==============================================================================
// 内部シェーダークラス (TrailShader)
//==============================================================================
class TrailShader : public Shader
{
public:
	// 入力レイアウトの定義
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

	// シェーダーのロード
	m_Shader = new TrailShader();
	m_Shader->Load(L"Shader/TrailVertexShader.hlsl", L"Shader/TrailPixelShader.hlsl");

	// テクスチャのロード (戻り値は ID3D11ShaderResourceView* です)
	m_TextureSRV = Texture::Load(texturePath);

	CreateVertexBuffer();
}

void TrailRenderer::CreateVertexBuffer()
{
	// 動的バッファとして作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(TrailVertex) * 2000; // 十分なサイズを確保
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

	// 1. 新しいポイントの追加
	Vector3 posVec = GetGameObject()->GetTransform()->position;
	XMFLOAT3 currentPos = { posVec.x, posVec.y, posVec.z };

	// Time::GetTime() は無いので GetTotalTime() を使用
	float currentTime = Time::GetTotalTime();

	bool shouldAdd = false;
	if (m_Points.empty())
	{
		shouldAdd = true;
	}
	else
	{
		XMFLOAT3 lastPos = m_Points.back().Position;
		// 距離チェック
		float distSq = (currentPos.x - lastPos.x) * (currentPos.x - lastPos.x) +
			(currentPos.y - lastPos.y) * (currentPos.y - lastPos.y) +
			(currentPos.z - lastPos.z) * (currentPos.z - lastPos.z);

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

	// 2. 寿命切れのポイントを削除
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
	Camera* camera = Camera::GetMain();
	if (!camera) return;

	// CameraはComponentなのでGameObject経由でTransformを取得
	Vector3 camPosVec = camera->GetGameObject()->GetTransform()->position;
	XMVECTOR camPos = XMLoadFloat3((XMFLOAT3*)&camPosVec);

	float currentTime = Time::GetTotalTime();

	// ポイント間を繋ぐリボンを生成
	for (size_t i = 0; i < m_Points.size(); ++i)
	{
		const auto& pt = m_Points[i];
		XMVECTOR pCurrent = XMLoadFloat3(&pt.Position);

		// 進行方向の計算
		XMVECTOR forward;
		if (i < m_Points.size() - 1)
		{
			XMVECTOR pNext = XMLoadFloat3(&m_Points[i + 1].Position);
			forward = XMVectorSubtract(pNext, pCurrent);
		}
		else
		{
			XMVECTOR pPrev = XMLoadFloat3(&m_Points[i - 1].Position);
			forward = XMVectorSubtract(pCurrent, pPrev);
		}
		forward = XMVector3Normalize(forward);

		// カメラへの方向
		XMVECTOR toCam = XMVectorSubtract(camPos, pCurrent);

		// ビルボード用の右ベクトル
		XMVECTOR right = XMVector3Cross(forward, toCam);
		right = XMVector3Normalize(right);

		// 寿命に応じた色の補間率
		float age = currentTime - pt.Time;
		float alphaRatio = 1.0f - (age / m_Lifetime);
		if (alphaRatio < 0) alphaRatio = 0;
		if (alphaRatio > 1) alphaRatio = 1;

		XMFLOAT4 color;
		color.x = m_StartColor.x * alphaRatio + m_EndColor.x * (1 - alphaRatio);
		color.y = m_StartColor.y * alphaRatio + m_EndColor.y * (1 - alphaRatio);
		color.z = m_StartColor.z * alphaRatio + m_EndColor.z * (1 - alphaRatio);
		color.w = m_StartColor.w * alphaRatio + m_EndColor.w * (1 - alphaRatio);

		// UV
		float u = static_cast<float>(i) / (m_Points.size() - 1);

		// 太さ調整 (先端ほど細く)
		float widthScale = alphaRatio;

		// 頂点生成
		TrailVertex v1, v2;
		XMVECTOR vOffset = XMVectorScale(right, m_Width * 0.5f * widthScale);

		// 上側
		XMStoreFloat3(&v1.Position, XMVectorAdd(pCurrent, vOffset));
		v1.Color = color;
		v1.TexCoord = { u, 0.0f };

		// 下側
		XMStoreFloat3(&v2.Position, XMVectorSubtract(pCurrent, vOffset));
		v2.Color = color;
		v2.TexCoord = { u, 1.0f };

		vertices.push_back(v2);
		vertices.push_back(v1);
	}

	// バッファ転送
	D3D11_MAPPED_SUBRESOURCE ms;
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();
	if (SUCCEEDED(context->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms)))
	{
		size_t dataSize = vertices.size() * sizeof(TrailVertex);
		memcpy(ms.pData, vertices.data(), dataSize);
		context->Unmap(m_VertexBuffer, 0);
	}
}

void TrailRenderer::Render()
{
	if (m_Points.size() < 2) return;

	UpdateVertexBuffer();

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// ステート設定: 修正ポイント
	// Renderer::SetDepthState は無いので、EnableとWriteEnableを個別に呼ぶ
	Renderer::SetDepthEnable(true);
	Renderer::SetDepthWriteEnable(false); // 半透明なので書き込みOFF

	// Renderer::SetBlendState は無いので、BlendModeを使用
	Renderer::SetBlendMode(BlendMode::Alpha);
	Renderer::SetCullingMode(false); // 両面描画

	// シェーダーセット
	m_Shader->Set();

	// 行列の設定
	// TrailVertexShaderでは View, Projection を使う
	// Trailの頂点はワールド座標系で作っているので、World行列は単位行列にする
	Renderer::SetWorldMatrix(XMMatrixIdentity());

	// View, Projectionは現在のカメラのものがRendererに設定済みと仮定できるが、
	// 念のためカメラから取得してセットするなら以下
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

	// 描画
	UINT stride = sizeof(TrailVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->Draw((UINT)m_Points.size() * 2, 0);

	// 設定を戻す
	Renderer::SetDepthWriteEnable(true);
	Renderer::SetCullingMode(true);
}
