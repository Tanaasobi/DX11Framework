#pragma once

//==============================================================================
// Puck.h - パック
//==============================================================================

#include "Core/Object/GameObject.h"
#include "Core/Graphics/Shader/IShader.h"

class CircleCollider;
class TrailRenderer;
class ParticleEmitter;
class ParticleShader;
class Model;

class Puck : public GameObject
{
public:
	Puck();
	virtual ~Puck();

	void Init(IShader* shader);
	void Update(float deltaTime) override;

	CircleCollider* GetCollider() const { return m_Collider; }

	void SetVelocity(float vx, float vz);
	void GetVelocity(float& outVX, float& outVZ) const;
	void Push(float pushX, float pushZ);

	float friction = 0.998f;
	float restitution = 0.9f;
	float minSpeed = 0.1f;

private:
	CircleCollider* m_Collider = nullptr;
	TrailRenderer* m_Trail = nullptr;
	Model* m_Model = nullptr; // モデルリソースの管理用

	ParticleEmitter* m_Sparks = nullptr; // 衝突エフェクト用のパーティクルエミッター
	ParticleShader* m_SparkShader = nullptr;

	float m_VelX = 0.0f;
	float m_VelZ = 0.0f;

	void ApplyFriction();
	void ReflectWalls();
};
