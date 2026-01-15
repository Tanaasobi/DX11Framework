//==============================================================================
// SpriteVertexShader.hlsl - スプライト用頂点シェーダー
//==============================================================================

cbuffer SpriteBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 Color;
    float4 UVOffset; // x, y = オフセット, z, w = スケール
}

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float4 Color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 pos = float4(input.Position, 1.0f);
    pos = mul(pos, World);
    pos = mul(pos, View);
    pos = mul(pos, Projection);

    output.Position = pos;
    
    // UVオフセット・スケール適用
    output.TexCoord = input.TexCoord * UVOffset.zw + UVOffset.xy;
    output.Color = Color;

    return output;
}
