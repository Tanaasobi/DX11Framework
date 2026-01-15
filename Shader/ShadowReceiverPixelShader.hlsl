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
    float ShadowBias;
    float ShadowIntensity;
    float2 ShadowPadding;
}

Texture2D g_Texture : register(t0);
Texture2D g_ShadowMap : register(t1);
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    float3 WorldPos : WORLD_POS;
};

float CalcShadow(float3 worldPos)
{
    // ワールド座標をライト空間に変換
    float4 lightSpacePos = mul(float4(worldPos, 1.0f), LightViewProj);
    
    // NDC座標に変換
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    // UV座標に変換（-1?1 → 0?1）
    float2 shadowUV;
    shadowUV.x = projCoords.x * 0.5f + 0.5f;
    shadowUV.y = -projCoords.y * 0.5f + 0.5f;
    
    // 範囲外チェック
    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f || shadowUV.y > 1.0f)
    {
        return 1.0f; // 影なし
    }
    
    // 深度チェック
    if (projCoords.z < 0.0f || projCoords.z > 1.0f)
    {
        return 1.0f; // 影なし
    }
    
    // シャドウマップから深度を取得
    float shadowDepth = g_ShadowMap.Sample(g_Sampler, shadowUV).r;
    float currentDepth = projCoords.z;
    
    // 影判定
    float shadow = (currentDepth - ShadowBias > shadowDepth) ? ShadowIntensity : 1.0f;
    
    return shadow;
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
