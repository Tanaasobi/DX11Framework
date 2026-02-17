#pragma once
#include "Shader.h"

//==============================================================================
// GaugeShader - ゲージ描画用シェーダー
//==============================================================================
class GaugeShader : public Shader
{
public:
	bool CreateInputLayout(ID3DBlob* vsBlob) override;
};
