//==============================================================================
// LightningTrail.cpp - 雷トレイルシステム実装
//==============================================================================

#include "LightningTrail.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"
#include "Core/Object/Camera.h"
#include <cmath>

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
LightningTrail::LightningTrail()
	: m_Rng(std::random_device{}())
{
}

//==============================================================================
// デストラクタ
//==============================================================================
LightningTrail::~LightningTrail()
{
	SAFE_RELEASE(m_VertexBuffer);
}

//==============================================================================
// 初期化
//==============================================================================
bool LightningTrail::Init(int maxSegments)
{
	m_MaxSegments = maxSegments;
	m_Segments.reserve(maxSegments);

	// 頂点バッファ作成（各セグメント = 6頂点）
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(LightningVertex) * maxSegments * 6;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_VertexBuffer);
	return SUCCEEDED(hr);
}

//==============================================================================
// 更新
//==============================================================================
void LightningTrail::Update(float deltaTime)
{
	// セグメント寿命更新
	for (auto it = m_Segments.begin(); it != m_Segments.end();)
	{
		it->life -= deltaTime;
		if (it->life <= 0.0f)
		{
			it = m_Segments.erase(it);
		}
		else
		{
			float t = it->life / it->lifeMax;
			it->alpha = t;
			++it;
		}
	}

	// 自動発生
	if (m_IsPlaying)
	{
		Vector3 currentPos = GetGameObject()->GetTransform()->position;
		EmitAuto(currentPos);
	}

	// 頂点バッファ更新
	UpdateVertexBuffer();
}

//==============================================================================
// 描画
//==============================================================================
void LightningTrail::Render()
{
	if (m_VertexCount == 0 || !m_Shader) return;

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// シェーダーセット
	m_Shader->Set();

	// ワールド行列（単位行列）
	XMMATRIX world = XMMatrixIdentity();
	Renderer::SetWorldMatrix(world);

	// テクスチャセット
	if (m_Texture)
	{
		context->PSSetShaderResources(0, 1, &m_Texture);
	}

	// 加算ブレンド・深度書き込み無効
	Renderer::SetBlendMode(BlendMode::Add);
	Renderer::SetDepthEnable(false);

	// 頂点バッファセット
	UINT stride = sizeof(LightningVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 描画
	context->Draw(m_VertexCount, 0);

	// 設定戻す
	Renderer::SetDepthEnable(true);
	Renderer::SetBlendMode(BlendMode::Alpha);
}

//==============================================================================
// 自動発生
//==============================================================================
void LightningTrail::EmitAuto(const Vector3& currentPos)
{
	if (!m_HasLastPosition)
	{
		m_LastPosition = currentPos;
		m_HasLastPosition = true;
		return;
	}

	float dx = currentPos.x - m_LastPosition.x;
	float dy = currentPos.y - m_LastPosition.y;
	float dz = currentPos.z - m_LastPosition.z;
	float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

	if (dist >= emitDistance)
	{
		Emit(m_LastPosition, currentPos);
		m_LastPosition = currentPos;
	}
}

//==============================================================================
// 雷発生
//==============================================================================
void LightningTrail::Emit(const Vector3& from, const Vector3& to)
{
	std::vector<Vector3> points;
	GenerateLightningPath(from, to, points);

	for (size_t i = 0; i < points.size() - 1; i++)
	{
		if (m_Segments.size() >= static_cast<size_t>(m_MaxSegments))
		{
			m_Segments.erase(m_Segments.begin());
		}

		LightningSegment seg;
		seg.start = points[i];
		seg.end = points[i + 1];
		seg.life = segmentLife;
		seg.lifeMax = segmentLife;
		seg.width = width;
		seg.alpha = 1.0f;

		m_Segments.push_back(seg);
	}
}

//==============================================================================
// ジグザグパス生成
//==============================================================================
void LightningTrail::GenerateLightningPath(const Vector3& from, const Vector3& to,
	std::vector<Vector3>& outPoints)
{
	outPoints.clear();
	outPoints.push_back(from);

	Vector3 dir;
	dir.x = to.x - from.x;
	dir.y = to.y - from.y;
	dir.z = to.z - from.z;

	// 垂直ベクトルを計算
	Vector3 up(0, 1, 0);
	Vector3 right;
	right.x = dir.y * up.z - dir.z * up.y;
	right.y = dir.z * up.x - dir.x * up.z;
	right.z = dir.x * up.y - dir.y * up.x;

	float rightLen = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
	if (rightLen > 0.001f)
	{
		right.x /= rightLen;
		right.y /= rightLen;
		right.z /= rightLen;
	}
	else
	{
		right = Vector3(1, 0, 0);
	}

	// もう一つの垂直ベクトル
	Vector3 up2;
	up2.x = dir.y * right.z - dir.z * right.y;
	up2.y = dir.z * right.x - dir.x * right.z;
	up2.z = dir.x * right.y - dir.y * right.x;

	float up2Len = std::sqrt(up2.x * up2.x + up2.y * up2.y + up2.z * up2.z);
	if (up2Len > 0.001f)
	{
		up2.x /= up2Len;
		up2.y /= up2Len;
		up2.z /= up2Len;
	}

	// 分割して中間点にジッターを加える
	for (int i = 1; i < subdivisions; i++)
	{
		float t = static_cast<float>(i) / subdivisions;

		float jitterRight = RandomRange(-jitter, jitter);
		float jitterUp = RandomRange(-jitter, jitter);

		Vector3 mid;
		mid.x = from.x + dir.x * t + right.x * jitterRight + up2.x * jitterUp;
		mid.y = from.y + dir.y * t + right.y * jitterRight + up2.y * jitterUp;
		mid.z = from.z + dir.z * t + right.z * jitterRight + up2.z * jitterUp;

		outPoints.push_back(mid);
	}

	outPoints.push_back(to);
}

//==============================================================================
// 頂点バッファ更新
//==============================================================================
void LightningTrail::UpdateVertexBuffer()
{
	if (m_Segments.empty())
	{
		m_VertexCount = 0;
		return;
	}

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = context->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr)) return;

	LightningVertex* vertices = static_cast<LightningVertex*>(mapped.pData);

	// カメラ位置取得
	Camera* camera = Camera::GetMain();
	XMFLOAT3 camPos(0, 10, -10);
	if (camera)
	{
		camPos = camera->GetGameObject()->GetTransform()->position.ToXMFLOAT3();
	}

	int vertexIndex = 0;

	for (const auto& seg : m_Segments)
	{
		// 線の方向
		XMFLOAT3 lineDir;
		lineDir.x = seg.end.x - seg.start.x;
		lineDir.y = seg.end.y - seg.start.y;
		lineDir.z = seg.end.z - seg.start.z;

		// 中点からカメラへの方向
		XMFLOAT3 midPoint;
		midPoint.x = (seg.start.x + seg.end.x) * 0.5f;
		midPoint.y = (seg.start.y + seg.end.y) * 0.5f;
		midPoint.z = (seg.start.z + seg.end.z) * 0.5f;

		XMFLOAT3 toCamera;
		toCamera.x = camPos.x - midPoint.x;
		toCamera.y = camPos.y - midPoint.y;
		toCamera.z = camPos.z - midPoint.z;

		// 外積でビルボード幅方向
		XMFLOAT3 widthDir;
		widthDir.x = lineDir.y * toCamera.z - lineDir.z * toCamera.y;
		widthDir.y = lineDir.z * toCamera.x - lineDir.x * toCamera.z;
		widthDir.z = lineDir.x * toCamera.y - lineDir.y * toCamera.x;

		float len = std::sqrt(widthDir.x * widthDir.x + widthDir.y * widthDir.y + widthDir.z * widthDir.z);
		if (len > 0.001f)
		{
			widthDir.x /= len;
			widthDir.y /= len;
			widthDir.z /= len;
		}

		float halfWidth = seg.width * 0.5f;

		// 色（アルファ適用）
		XMFLOAT4 color = colorCore;
		color.w = colorCore.w * seg.alpha;

		// 4頂点
		XMFLOAT3 p0, p1, p2, p3;
		p0.x = seg.start.x - widthDir.x * halfWidth;
		p0.y = seg.start.y - widthDir.y * halfWidth;
		p0.z = seg.start.z - widthDir.z * halfWidth;

		p1.x = seg.start.x + widthDir.x * halfWidth;
		p1.y = seg.start.y + widthDir.y * halfWidth;
		p1.z = seg.start.z + widthDir.z * halfWidth;

		p2.x = seg.end.x - widthDir.x * halfWidth;
		p2.y = seg.end.y - widthDir.y * halfWidth;
		p2.z = seg.end.z - widthDir.z * halfWidth;

		p3.x = seg.end.x + widthDir.x * halfWidth;
		p3.y = seg.end.y + widthDir.y * halfWidth;
		p3.z = seg.end.z + widthDir.z * halfWidth;

		// 三角形1
		vertices[vertexIndex++] = { p0, color, { 0.0f, 0.0f } };
		vertices[vertexIndex++] = { p1, color, { 1.0f, 0.0f } };
		vertices[vertexIndex++] = { p2, color, { 0.0f, 1.0f } };

		// 三角形2
		vertices[vertexIndex++] = { p1, color, { 1.0f, 0.0f } };
		vertices[vertexIndex++] = { p3, color, { 1.0f, 1.0f } };
		vertices[vertexIndex++] = { p2, color, { 0.0f, 1.0f } };
	}

	context->Unmap(m_VertexBuffer, 0);

	m_VertexCount = vertexIndex;
}

//==============================================================================
// ランダム範囲
//==============================================================================
float LightningTrail::RandomRange(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(m_Rng);
}
