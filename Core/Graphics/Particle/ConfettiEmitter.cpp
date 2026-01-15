//==============================================================================
// ConfettiEmitter.cpp - 紙吹雪エミッター実装
//==============================================================================

#include "ConfettiEmitter.h"
#include "Core/Graphics/Renderer.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"
#include "Core/Object/Camera.h"
#include <cmath>

using namespace DirectX;

//==============================================================================
// コンストラクタ
//==============================================================================
ConfettiEmitter::ConfettiEmitter()
	: m_Rng(std::random_device{}())
{
	// カラーパレット
	m_Colors = {
		{ 1.0f, 0.2f, 0.2f, 1.0f },  // 赤
		{ 0.2f, 0.8f, 0.2f, 1.0f },  // 緑
		{ 0.2f, 0.4f, 1.0f, 1.0f },  // 青
		{ 1.0f, 0.9f, 0.2f, 1.0f },  // 黄
		{ 1.0f, 0.5f, 0.0f, 1.0f },  // オレンジ
		{ 0.8f, 0.2f, 1.0f, 1.0f },  // 紫
		{ 1.0f, 0.4f, 0.7f, 1.0f },  // ピンク
		{ 0.2f, 1.0f, 1.0f, 1.0f },  // シアン
	};
}

//==============================================================================
// デストラクタ
//==============================================================================
ConfettiEmitter::~ConfettiEmitter()
{
	SAFE_RELEASE(m_VertexBuffer);
}

//==============================================================================
// 初期化
//==============================================================================
bool ConfettiEmitter::Init(int maxParticles)
{
	m_MaxParticles = maxParticles;
	m_Particles.resize(maxParticles);

	for (auto& p : m_Particles)
	{
		p.active = false;
	}

	// 頂点バッファ作成（各パーティクル = 6頂点）
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(ConfettiVertex) * maxParticles * 6;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_VertexBuffer);
	return SUCCEEDED(hr);
}

//==============================================================================
// 更新
//==============================================================================
void ConfettiEmitter::Update(float deltaTime)
{ 
	for (auto& p : m_Particles)
	{
		if (!p.active) continue;

		// 寿命更新
		p.life -= deltaTime;
		if (p.life <= 0.0f)
		{
			p.active = false;
			continue;
		}

		// 重力
		p.velocity.y -= gravity * deltaTime;

		// 左右の揺れ（サイン波）
		p.swayPhase += p.swaySpeed * deltaTime;
		float sway = std::sin(p.swayPhase) * p.swayAmount;

		// 位置更新
		p.position.x += p.velocity.x * deltaTime + sway * deltaTime;
		p.position.y += p.velocity.y * deltaTime;
		p.position.z += p.velocity.z * deltaTime;

		// 回転更新（ひらひら感）
		p.rotationX += p.rotationSpeedX * deltaTime;
		p.rotationY += p.rotationSpeedY * deltaTime;
		p.rotationZ += p.rotationSpeedZ * deltaTime;

		// 空気抵抗（水平方向の減衰）
		p.velocity.x *= 0.99f;
		p.velocity.z *= 0.99f;

		// 落下速度制限（ターミナルベロシティ）
		if (p.velocity.y < -5.0f)
		{
			p.velocity.y = -5.0f;
		}
	}

	UpdateVertexBuffer();
}

//==============================================================================
// 描画
//==============================================================================
void ConfettiEmitter::Render()
{
	if (m_VertexCount == 0 || !m_Shader) return;

	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	m_Shader->Set();

	// ワールド行列（単位行列）
	XMMATRIX world = XMMatrixIdentity();
	Renderer::SetWorldMatrix(world);

	if (m_Texture)
	{
		context->PSSetShaderResources(0, 1, &m_Texture);
	}

	// アルファブレンド
	Renderer::SetBlendMode(BlendMode::Alpha);
	Renderer::SetDepthEnable(true);
	Renderer::SetDepthWriteEnable(false);

	// 両面描画
	Renderer::SetCullingMode(false);

	UINT stride = sizeof(ConfettiVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->Draw(m_VertexCount, 0);

	// 設定戻す
	Renderer::SetCullingMode(true);
	Renderer::SetDepthWriteEnable(true);
}

//==============================================================================
// バースト発生
//==============================================================================
void ConfettiEmitter::Burst(int count, const Vector3& position)
{
	int spawned = 0;

	for (auto& p : m_Particles)
	{
		if (spawned >= count) break;
		if (p.active) continue;

		p.active = true;

		// 位置（少しばらつかせる）
		p.position.x = position.x + RandomRange(-spreadX * 0.5f, spreadX * 0.5f);
		p.position.y = position.y + RandomRange(0, spreadY);
		p.position.z = position.z + RandomRange(-spreadZ * 0.5f, spreadZ * 0.5f);

		// 速度（上方向に放出）
		float speed = RandomRange(initialSpeedMin, initialSpeedMax);
		float angleH = RandomRange(0.0f, 3.14159f * 2.0f);
		float angleV = RandomRange(0.3f, 0.8f);  // 上向き

		p.velocity.x = std::cos(angleH) * speed * (1.0f - angleV);
		p.velocity.y = speed * angleV;
		p.velocity.z = std::sin(angleH) * speed * (1.0f - angleV);

		// 回転（ランダム）
		p.rotationX = RandomRange(0, 360);
		p.rotationY = RandomRange(0, 360);
		p.rotationZ = RandomRange(0, 360);
		p.rotationSpeedX = RandomRange(-360, 360);
		p.rotationSpeedY = RandomRange(-360, 360);
		p.rotationSpeedZ = RandomRange(-540, 540);  // Z軸は速く

		// サイズ
		p.size = RandomRange(sizeMin, sizeMax);

		// 寿命
		p.life = RandomRange(lifeMin, lifeMax);
		p.lifeMax = p.life;

		// 揺れ
		p.swayPhase = RandomRange(0, 6.28f);
		p.swaySpeed = RandomRange(3.0f, 6.0f);
		p.swayAmount = RandomRange(1.0f, 3.0f);

		// 色
		p.color = RandomColor();

		spawned++;
	}
}

//==============================================================================
// バースト発生（方向指定版）
//==============================================================================
void ConfettiEmitter::Burst(int count, const Vector3& position, const Vector3& direction)
{
	int spawned = 0;

	for (auto& p : m_Particles)
	{
		if (spawned >= count) break;
		if (p.active) continue;

		p.active = true;

		// 位置（少しばらつかせる）
		p.position.x = position.x + RandomRange(-spreadX * 0.3f, spreadX * 0.3f);
		p.position.y = position.y + RandomRange(0, spreadY);
		p.position.z = position.z + RandomRange(-spreadZ * 0.3f, spreadZ * 0.3f);

		// 速度（指定方向を基準に放出）
		float speed = RandomRange(initialSpeedMin, initialSpeedMax);

		// 方向にランダムな散らばりを加える
		float spreadAngleH = RandomRange(-0.5f, 0.5f);  // 水平方向の散らばり
		float spreadAngleV = RandomRange(-0.3f, 0.2f);  // 垂直方向の散らばり（上向き多め）

		// 基本方向
		float baseX = direction.x;
		float baseZ = direction.z;

		// 水平方向に回転（散らばり）
		float cosH = std::cos(spreadAngleH);
		float sinH = std::sin(spreadAngleH);
		float rotatedX = baseX * cosH - baseZ * sinH;
		float rotatedZ = baseX * sinH + baseZ * cosH;

		p.velocity.x = rotatedX * speed;
		p.velocity.y = speed * (0.5f + spreadAngleV);  // 上向き成分
		p.velocity.z = rotatedZ * speed;

		// 回転（ランダム）
		p.rotationX = RandomRange(0, 360);
		p.rotationY = RandomRange(0, 360);
		p.rotationZ = RandomRange(0, 360);
		p.rotationSpeedX = RandomRange(-360, 360);
		p.rotationSpeedY = RandomRange(-360, 360);
		p.rotationSpeedZ = RandomRange(-540, 540);

		// サイズ
		p.size = RandomRange(sizeMin, sizeMax);

		// 寿命
		p.life = RandomRange(lifeMin, lifeMax);
		p.lifeMax = p.life;

		// 揺れ
		p.swayPhase = RandomRange(0, 6.28f);
		p.swaySpeed = RandomRange(3.0f, 6.0f);
		p.swayAmount = RandomRange(1.0f, 3.0f);

		// 色
		p.color = RandomColor();

		spawned++;
	}
}

//==============================================================================
// 頂点バッファ更新
//==============================================================================
void ConfettiEmitter::UpdateVertexBuffer()
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = context->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr)) return;

	ConfettiVertex* vertices = static_cast<ConfettiVertex*>(mapped.pData);

	// カメラ取得
	Camera* camera = Camera::GetMain();
	XMFLOAT3 camPos(0, 10, -10);
	if (camera)
	{
		camPos = camera->GetGameObject()->GetTransform()->position.ToXMFLOAT3();
	}

	int vertexIndex = 0;

	for (const auto& p : m_Particles)
	{
		if (!p.active) continue;

		// 回転行列
		float radX = p.rotationX * (3.14159f / 180.0f);
		float radY = p.rotationY * (3.14159f / 180.0f);
		float radZ = p.rotationZ * (3.14159f / 180.0f);

		XMMATRIX rotMatrix = XMMatrixRotationRollPitchYaw(radX, radY, radZ);

		// ローカル座標の4隅（平べったい長方形）
		float hw = p.size * 0.5f;
		float hh = p.size * 0.3f;  // 縦長にして紙っぽく

		XMFLOAT3 localCorners[4] = {
			{ -hw, -hh, 0 },
			{  hw, -hh, 0 },
			{ -hw,  hh, 0 },
			{  hw,  hh, 0 }
		};

		XMFLOAT3 worldCorners[4];
		for (int i = 0; i < 4; i++)
		{
			XMVECTOR v = XMLoadFloat3(&localCorners[i]);
			v = XMVector3TransformCoord(v, rotMatrix);
			XMStoreFloat3(&worldCorners[i], v);

			worldCorners[i].x += p.position.x;
			worldCorners[i].y += p.position.y;
			worldCorners[i].z += p.position.z;
		}

		// フェードアウト
		float alpha = p.life / p.lifeMax;
		XMFLOAT4 color = p.color;
		color.w = alpha;

		// 三角形1
		vertices[vertexIndex++] = { worldCorners[0], color, { 0, 1 } };
		vertices[vertexIndex++] = { worldCorners[2], color, { 0, 0 } };
		vertices[vertexIndex++] = { worldCorners[1], color, { 1, 1 } };

		// 三角形2
		vertices[vertexIndex++] = { worldCorners[1], color, { 1, 1 } };
		vertices[vertexIndex++] = { worldCorners[2], color, { 0, 0 } };
		vertices[vertexIndex++] = { worldCorners[3], color, { 1, 0 } };
	}

	context->Unmap(m_VertexBuffer, 0);

	m_VertexCount = vertexIndex;
}

//==============================================================================
// ランダム範囲
//==============================================================================
float ConfettiEmitter::RandomRange(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(m_Rng);
}

//==============================================================================
// ランダムカラー
//==============================================================================
XMFLOAT4 ConfettiEmitter::RandomColor()
{
	std::uniform_int_distribution<int> dist(0, static_cast<int>(m_Colors.size()) - 1);
	return m_Colors[dist(m_Rng)];
}
