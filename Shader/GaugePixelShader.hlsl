// Shader/GaugePixelShader.hlsl

cbuffer GaugeBuffer : register(b2)
{
    float4 Color; // ゲージの色
    float Progress; // 進捗 (0.0 ～ 1.0)
    float InnerRadius; // 内径
    float2 Padding;
}

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 中心を (0,0) とするUV座標
    float2 uv = input.TexCoord - 0.5f;
    float dist = length(uv);
    
    // 円形クリッピング (外径0.5より外、または内径より内側なら描画しない)
    if (dist > 0.5f || dist < InnerRadius)
    {
        discard;
    }
    
    // 角度計算 (時計回り)
    float angle = atan2(uv.x, -uv.y);
    if (angle < 0.0f)
        angle += 6.283185307f;
    
    float currentProgress = angle / 6.283185307f;
    
    // 進捗チェック
    if (currentProgress > Progress)
    {
        // discard せずに、薄い色（背景色）を返す
        // アルファ値を下げて薄く表示 (例: 20%の濃さ)
        return float4(Color.rgb, Color.a * 0.2f);
    }
    
    return Color;
}
