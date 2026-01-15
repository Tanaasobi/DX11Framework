//==============================================================================
// LightningVertexShader.hlsl
//==============================================================================

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
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 worldPos = float4(input.Position, 1.0f);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    output.Color = input.Color;
    output.TexCoord = input.TexCoord;

    return output;
}
