//==============================================================================
// ParticleEmitter.cpp - パーティクルエミッターコンポーネント実装
//==============================================================================

#include "ParticleEmitter.h"
#include "Core/Object/GameObject.h"
#include "Core/Object/Transform.h"
#include "Core/Graphics/Texture.h"

ParticleEmitter::ParticleEmitter()
	: m_Rng(std::random_device{}())
{
}

ParticleEmitter::~ParticleEmitter()
{
	m_Pool.Uninit();
	m_Renderer.Uninit();
}

bool ParticleEmitter::Init(int maxParticles)
{
	settings.maxParticles = maxParticles;
	m_Pool.Init(maxParticles);

	if (!m_Renderer.Init(maxParticles))
	{
		return false;
	}

	return true;
}

void ParticleEmitter::Update(float deltaTime)
{
	// パーティクル発生
	if (m_IsPlaying && settings.emitRate > 0.0f)
	{
		m_EmitAccumulator += settings.emitRate * deltaTime;

		while (m_EmitAccumulator >= 1.0f)
		{
			EmitParticle();
			m_EmitAccumulator -= 1.0f;
		}
	}

	// パーティクル更新
	m_Pool.Update(deltaTime);

	// インスタンスバッファ更新
	m_Renderer.UpdateInstanceBuffer(m_Pool.GetParticles());
}

void ParticleEmitter::Render()
{
	if (!m_Shader) return;

	m_Renderer.SetShader(m_Shader);
	m_Renderer.SetTexture(m_Texture);
	m_Renderer.SetAdditiveBlend(settings.additiveBlend);
	m_Renderer.Render(m_Pool.GetActiveCount());
}

void ParticleEmitter::EmitParticle()
{
	Particle* p = m_Pool.Emit();
	if (!p) return;

	// エミッター位置
	Transform* transform = GetGameObject()->GetTransform();
	p->position = transform->position;

	// ランダム設定
	p->velocity.x = RandomRange(settings.velocityMin.x, settings.velocityMax.x);
	p->velocity.y = RandomRange(settings.velocityMin.y, settings.velocityMax.y);
	p->velocity.z = RandomRange(settings.velocityMin.z, settings.velocityMax.z);

	p->acceleration = settings.acceleration;

	p->life = RandomRange(settings.lifeMin, settings.lifeMax);
	p->lifeMax = p->life;

	p->sizeStart = settings.sizeStart;
	p->sizeEnd = settings.sizeEnd;
	p->size = p->sizeStart;

	p->rotation = RandomRange(settings.rotationMin, settings.rotationMax);
	p->rotationSpeed = RandomRange(settings.rotationSpeedMin, settings.rotationSpeedMax);

	p->colorStart = settings.colorStart;
	p->colorEnd = settings.colorEnd;
	p->color = p->colorStart;
}

void ParticleEmitter::Burst(int count)
{
	for (int i = 0; i < count; i++)
	{
		EmitParticle();
	}
}

void ParticleEmitter::Burst(int count, const Vector3& position, const Vector3& direction, float speed, float spread)
{
	for (int i = 0; i < count; i++)
	{
		Particle* p = m_Pool.Emit();
		if (!p) return;

		// 指定された位置にセット
		p->position = position;

		// 方向の計算（指定方向 + ランダムな拡散）
		Vector3 randomDir;
		randomDir.x = RandomRange(-1.0f, 1.0f);
		randomDir.y = RandomRange(-1.0f, 1.0f) * spread; // 垂直方向への拡散量を調整
		randomDir.z = RandomRange(-1.0f, 1.0f);

		// directionが (0,0,0) でないことを前提とします
		Vector3 finalDir = direction + (randomDir * spread);
		finalDir.Normalize();

		// 速度設定（指定速度に少しばらつきを持たせる）
		float finalSpeed = speed * RandomRange(0.8f, 1.5f);
		p->velocity = finalDir * finalSpeed;

		// その他のパラメータは settings からコピー
		p->acceleration = settings.acceleration;

		p->life = RandomRange(settings.lifeMin, settings.lifeMax);
		p->lifeMax = p->life;

		p->sizeStart = settings.sizeStart;
		p->sizeEnd = settings.sizeEnd;
		p->size = p->sizeStart;

		p->rotation = RandomRange(settings.rotationMin, settings.rotationMax);
		p->rotationSpeed = RandomRange(settings.rotationSpeedMin, settings.rotationSpeedMax);

		p->colorStart = settings.colorStart;
		p->colorEnd = settings.colorEnd;
		p->color = p->colorStart;
	}
}

float ParticleEmitter::RandomRange(float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(m_Rng);
}

void ParticleEmitter::SetTexture(const std::string& fileName)
{
	m_Texture = Texture::Load(fileName);
}

void ParticleEmitter::SetTexture(ID3D11ShaderResourceView* texture)
{
	m_Texture = texture;
}

void ParticleEmitter::SetShader(ParticleShader* shader)
{
	m_Shader = shader;
}
