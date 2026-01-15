//==============================================================================
// ConfettiPixelShader.hlsl
//==============================================================================

Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_Texture.Sample(g_Sampler, input.TexCoord);
    float4 result = texColor * input.Color;
    
    // アルファが低すぎたらディスカード
    if (result.a < 0.1f)
        discard;
    
    return result;
}
