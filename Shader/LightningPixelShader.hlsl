//==============================================================================
// LightningPixelShader.hlsl
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
    
    // グロー効果を強調
    float4 result = texColor * input.Color;
    result.rgb *= 3.0f; // もっと明るく！
    
    // 白飛びを防ぐ（HDR風）
    result.rgb = result.rgb / (result.rgb + 1.0f) * 2.0f;
    
    return result;
}
