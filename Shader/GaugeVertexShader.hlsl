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
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    
    // ワールド座標変換
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    // ビュー変換
    float4 viewPos = mul(worldPos, View);
    // プロジェクション変換
    output.Position = mul(viewPos, Projection);
    
    output.TexCoord = input.TexCoord;
    
    return output;
}
