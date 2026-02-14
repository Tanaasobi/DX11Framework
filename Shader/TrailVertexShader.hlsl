// フレームワークの仕様に合わせてバッファを分割
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
    
    // 座標変換
    // WorldはC++側で単位行列を入れているはずだが、形式上一応掛ける
    matrix wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    output.Position = mul(float4(input.Position, 1.0f), wvp);
    
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    
    return output;
}
