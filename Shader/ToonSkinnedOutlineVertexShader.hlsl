//==============================================================================
// ToonSkinnedOutlineVertexShader.hlsl - スキニング対応アウトライン用頂点シェーダー
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

cbuffer BoneBuffer : register(b4)
{
    matrix BoneMatrices[256];
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
    uint4 BoneIndices : BLENDINDICES;
    float4 BoneWeights : BLENDWEIGHT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
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

    float4 skinnedPos = mul(float4(input.Position, 1.0f), skinMatrix);
    float3 skinnedNormal = normalize(mul(float4(input.Normal, 0.0f), skinMatrix).xyz);

    // 法線方向に押し出し
    skinnedPos.xyz += skinnedNormal * outline_Width;

    float4 worldPos = mul(skinnedPos, World);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    output.Color = outline_Color;

    return output;
}
