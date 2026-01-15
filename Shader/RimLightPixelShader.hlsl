//==============================================================================
// RimLightPixelShader.hlsl - リムライト付きピクセルシェーダー
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

cbuffer CameraBuffer : register(b6)
{
    float4 CameraPosition;
}

cbuffer RimLightBuffer : register(b7)
{
    float4 RimColor;
    float RimPower;
    float RimIntensity;
    float2 RimPadding;
}

Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    float3 WorldPos : WORLD_POS;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 基本色
    float4 color = input.Color * mat_Diffuse;
    
    if (mat_TextureEnable)
    {
        color *= g_Texture.Sample(g_Sampler, input.TexCoord);
    }
    
    //--------------------------------------------------------------------------
    // リムライト計算
    //--------------------------------------------------------------------------
    float3 N = normalize(input.Normal);
    float3 V = normalize(CameraPosition.xyz - input.WorldPos);
    
    // 視線と法線の角度が90度に近いほど強く光る
    float rim = 1.0f - saturate(dot(N, V));
    rim = pow(rim, RimPower) * RimIntensity;
    
    // リムライトを加算
    color.rgb += RimColor.rgb * rim;
    
    return color;
}
