//==============================================================================
// OutlineVertexShader.hlsl - アウトライン用頂点シェーダー
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

cbuffer OutlineBuffer : register(b7)
{
    float4 outline_Color;
    float outline_Width;
    float3 outline_Padding;
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
    float4 Color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // 法線方向に頂点を押し出す
    float3 pos = input.Position + input.Normal * outline_Width;

    float4 worldPos = mul(float4(pos, 1.0f), World);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    output.Color = outline_Color;

    return output;
}
