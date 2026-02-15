//==============================================================================
// TrailVertexShader.hlsl
//==============================================================================

// フレームワークのRenderer仕様に合わせてバッファを分割 (b0, b1, b2)
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
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    // トレイルの頂点はC++側で既にワールド座標として計算済み
    // World行列は単位行列のはずだが、形式的に掛けておく
    matrix wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    output.Position = mul(float4(input.Position, 1.0f), wvp);
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    
    return output;
}
