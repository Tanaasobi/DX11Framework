//==============================================================================
// RimLightSkinnedVertexShader.hlsl - リムライト用スキニング頂点シェーダー
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

// ボーン行列
cbuffer BoneBuffer : register(b4)
{
    matrix BoneMatrices[256];
}

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    uint4 BoneIndices : BLENDINDICES;
    float4 BoneWeights : BLENDWEIGHT;
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

    // スキニング計算
    matrix skinMatrix =
        BoneMatrices[input.BoneIndices.x] * input.BoneWeights.x +
        BoneMatrices[input.BoneIndices.y] * input.BoneWeights.y +
        BoneMatrices[input.BoneIndices.z] * input.BoneWeights.z +
        BoneMatrices[input.BoneIndices.w] * input.BoneWeights.w;

    // スキニング適用
    float4 skinnedPos = mul(float4(input.Position, 1.0f), skinMatrix);
    float3 skinnedNormal = mul(float4(input.Normal, 0.0f), skinMatrix).xyz;

    // ワールド変換
    float4 worldPos = mul(skinnedPos, World);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    output.Normal = normalize(mul(float4(skinnedNormal, 0.0f), World).xyz);
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    output.WorldPos = worldPos.xyz;

    return output;
}
