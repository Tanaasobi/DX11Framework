Texture2D mainTexture : register(t0);
SamplerState mainSampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = mainTexture.Sample(mainSampler, input.TexCoord);
    
    // テクスチャの色 * 頂点カラー（フェードアウト用）
    return texColor * input.Color;
}
