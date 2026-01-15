//==============================================================================
// ShadowReceiverPixelShader.hlsl - 影を受けるピクセルシェーダー
//==============================================================================

cbuffer MaterialBuffer : register(b3)
{
    float4 mat_Ambient;
    float4 mat_Diffuse;
    float4 mat_Specular;
    float4 mat_Emission;
    float mat_Shininess;
    bool mat_TextureEnable;
    float2 mat_Padding;
}

cbuffer LightBuffer : register(b5)
{
    bool light_Enable;
    float3 light_Padding;
    float4 light_Direction;
    float4 light_Diffuse;
    float4 light_Ambient;
}

cbuffer ShadowBuffer : register(b8)
{
    matrix LightViewProj;
    float4 ShadowParams; // x: bias, y: intensity
}

Texture2D g_Texture : register(t0);
Texture2D g_ShadowMap : register(t1);
SamplerState g_Sampler : register(s0);
SamplerComparisonState g_ShadowSampler : register(s1);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 WorldPos : WORLD_POS;
};

float CalcShadow(float3 worldPos)
{
    // ワールド座標をライト空間に変換
    float4 lightSpacePos = mul(float4(worldPos, 1.0f), LightViewProj);
    
    // NDC座標に変換
    lightSpacePos.xyz /= lightSpacePos.w;
    
    // UV座標に変換（-1?1 → 0?1）
    float2 shadowUV;
    shadowUV.x = lightSpacePos.x * 0.5f + 0.5f;
    shadowUV.y = -lightSpacePos.y * 0.5f + 0.5f;
    
    // 範囲外チェック
    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f || shadowUV.y > 1.0f)
    {
        return 1.0f; // 影なし
    }
    
    // 深度比較
    float currentDepth = lightSpacePos.z;
    float bias = ShadowParams.x;
    
    // PCF（ソフトシャドウ）
    float shadow = 0.0f;
    float texelSize = 1.0f / 2048.0f;
    
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float2 offset = float2(x, y) * texelSize;
            float shadowDepth = g_ShadowMap.Sample(g_Sampler, shadowUV + offset).r;
            shadow += (currentDepth - bias > shadowDepth) ? 0.0f : 1.0f;
        }
    }
    shadow /= 9.0f;
    
    // 影の強さを調整
    float intensity = ShadowParams.y;
    return lerp(intensity, 1.0f, shadow);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    // 基本色
    float4 color = input.Color * mat_Diffuse;
    
    if (mat_TextureEnable)
    {
        color *= g_Texture.Sample(g_Sampler, input.TexCoord);
    }
    
    // 影を計算
    float shadow = CalcShadow(input.WorldPos);
    
    // 影を適用
    color.rgb *= shadow;
    
    return color;
}
