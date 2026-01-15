//==============================================================================
// VertexShader.hlsl - 基本頂点シェーダー
//==============================================================================

// 定数バッファ
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

// 入力構造体
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

// 出力構造体
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

// メイン関数
VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // ワールド・ビュー・プロジェクション変換
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    float4 viewPos = mul(worldPos, View);
    output.Position = mul(viewPos, Projection);

    // 法線のワールド変換
    output.Normal = mul(float4(input.Normal, 0.0f), World).xyz;

    // 頂点カラーとテクスチャ座標はそのまま渡す
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;

    return output;
}
