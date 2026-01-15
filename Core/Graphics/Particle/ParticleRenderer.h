#pragma once

//==============================================================================
// ParticleRenderer.h - GPUインスタンシング描画
//==============================================================================

#include "ParticleSystem.h"
#include "Core/Graphics/Shader/IShader.h"
#include "Core/Graphics/Texture.h"

//==============================================================================
// ParticleRenderer クラス
//==============================================================================
class ParticleRenderer
{
public:
	ParticleRenderer();
	~ParticleRenderer();

	bool Init(int maxParticles);
	void Uninit();

	// インスタンスバッファ更新
	void UpdateInstanceBuffer(const std::vector<Particle>& particles);

	// 描画
	void Render(int activeCount);

	// 設定
	void SetShader(IShader* shader) { m_Shader = shader; }
	void SetTexture(ID3D11ShaderResourceView* texture) { m_Texture = texture; }
	void SetAdditiveBlend(bool additive) { m_AdditiveBlend = additive; }

private:
	// 頂点バッファ（ビルボード四角形）
	ID3D11Buffer* m_VertexBuffer = nullptr;

	// インスタンスバッファ
	ID3D11Buffer* m_InstanceBuffer = nullptr;
	int m_MaxParticles = 0;

	IShader* m_Shader = nullptr;
	ID3D11ShaderResourceView* m_Texture = nullptr;
	bool m_AdditiveBlend = true;

	bool CreateVertexBuffer();
	bool CreateInstanceBuffer(int maxParticles);
};
