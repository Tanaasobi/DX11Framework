//==============================================================================
// FieldVertexShader.hlsl - フィールド用頂点シェーダー
//==============================================================================

cbuffer WorldBuffer : register(b0)
{
    matrix World;
}

cbuffer ViewBuffer : register(b1)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    float3 WorldPos : WORLD_POS;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    output.Normal = normalize(mul(float4(input.Normal, 0.0f), World).xyz);
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    output.WorldPos = worldPos.xyz;

    return output;
}
