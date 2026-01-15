//==============================================================================
// ParticlePixelShader.hlsl - パーティクル用ピクセルシェーダー
//==============================================================================

Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float4 Color : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_Texture.Sample(g_Sampler, input.TexCoord);
    return texColor * input.Color;
}
