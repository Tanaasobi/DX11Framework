//==============================================================================
// ParticleSystem.cpp - パーティクルシステム実装
//==============================================================================

#include "ParticleSystem.h"
#include <algorithm>

//==============================================================================
// ParticlePool
//==============================================================================
ParticlePool::ParticlePool()
{
}

ParticlePool::~ParticlePool()
{
	Uninit();
}

void ParticlePool::Init(int maxParticles)
{
	m_Particles.resize(maxParticles);
	for (auto& p : m_Particles)
	{
		p.active = false;
	}
	m_ActiveCount = 0;
	m_NextIndex = 0;
}

void ParticlePool::Uninit()
{
	m_Particles.clear();
	m_ActiveCount = 0;
}

Particle* ParticlePool::Emit()
{
	// 空きパーティクルを探す
	int startIndex = m_NextIndex;
	int maxParticles = static_cast<int>(m_Particles.size());

	for (int i = 0; i < maxParticles; i++)
	{
		int index = (startIndex + i) % maxParticles;
		if (!m_Particles[index].active)
		{
			m_Particles[index].active = true;
			m_NextIndex = (index + 1) % maxParticles;
			m_ActiveCount++;
			return &m_Particles[index];
		}
	}

	// 空きがない場合は最も古いものを再利用
	m_Particles[m_NextIndex].active = true;
	Particle* p = &m_Particles[m_NextIndex];
	m_NextIndex = (m_NextIndex + 1) % maxParticles;
	return p;
}

void ParticlePool::Update(float deltaTime)
{
	m_ActiveCount = 0;

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

		m_ActiveCount++;

		// 物理更新
		p.velocity.x += p.acceleration.x * deltaTime;
		p.velocity.y += p.acceleration.y * deltaTime;
		p.velocity.z += p.acceleration.z * deltaTime;

		p.position.x += p.velocity.x * deltaTime;
		p.position.y += p.velocity.y * deltaTime;
		p.position.z += p.velocity.z * deltaTime;

		p.rotation += p.rotationSpeed * deltaTime;

		// 補間計算
		float t = 1.0f - (p.life / p.lifeMax);

		// サイズ補間
		p.size = p.sizeStart + (p.sizeEnd - p.sizeStart) * t;

		// 色補間
		p.color.x = p.colorStart.x + (p.colorEnd.x - p.colorStart.x) * t;
		p.color.y = p.colorStart.y + (p.colorEnd.y - p.colorStart.y) * t;
		p.color.z = p.colorStart.z + (p.colorEnd.z - p.colorStart.z) * t;
		p.color.w = p.colorStart.w + (p.colorEnd.w - p.colorStart.w) * t;
	}
}
