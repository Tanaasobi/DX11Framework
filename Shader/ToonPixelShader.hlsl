//==============================================================================
// ToonPixelShader.hlsl - トゥーンシェーダー
//==============================================================================

cbuffer MaterialBuffer : register(b3)
{
    float4 mat_Ambient;
    float4 mat_Diffuse;
    float4 mat_Specular;
    float4 mat_Emission;
    float mat_Shininess;
    int mat_TextureEnable;
    float2 mat_Padding;
}

cbuffer LightBuffer : register(b5)
{
    float4 light_Direction;
    float4 light_Diffuse;
    float4 light_Ambient;
}

cbuffer CameraBuffer : register(b6)
{
    float4 CameraPosition;
}

cbuffer ToonBuffer : register(b7)
{
    int toon_Levels; // 階調数（2?5程度）
    float toon_Edge; // エッジの鮮明さ
    float toon_RimPower; // リムライトの強さ
    float toon_RimIntensity; // リムライトの明るさ
    float4 toon_RimColor; // リムライトの色
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

// 階調化関数
float ToonStep(float value, int levels)
{
    return floor(value * levels) / (levels - 1);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    // 法線を正規化
    float3 normal = normalize(input.Normal);

    // ライト方向
    float3 lightDir = normalize(-light_Direction.xyz);

    // ディフューズ計算（Lambert）
    float NdotL = max(dot(normal, lightDir), 0.0f);

    // トゥーン階調化
    float toonDiffuse = ToonStep(NdotL, toon_Levels);

    // ディフューズ色
    float4 diffuse = light_Diffuse * mat_Diffuse * toonDiffuse;

    // アンビエント
    float4 ambient = light_Ambient * mat_Ambient;

    // リムライト計算
    float3 viewDir = normalize(CameraPosition.xyz - input.WorldPos);
    float rim = 1.0f - saturate(dot(normal, viewDir));
    rim = pow(rim, toon_RimPower) * toon_RimIntensity;
    float4 rimColor = toon_RimColor * rim;

    // 最終色
    float4 color = ambient + diffuse + rimColor;
    color.a = mat_Diffuse.a;

    // 頂点カラー
    color *= input.Color;

    // テクスチャ
    if (mat_TextureEnable)
    {
        float4 texColor = g_Texture.Sample(g_Sampler, input.TexCoord);
        color *= texColor;
    }

    return color;
}
