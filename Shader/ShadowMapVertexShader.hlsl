//==============================================================================
// ShadowMapVertexShader.hlsl - シャドウマップ用頂点シェーダー（スキニング対応）
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

    // ワールド → ビュー → プロジェクション
    float4 worldPos = mul(skinnedPos, World);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    return output;
}
