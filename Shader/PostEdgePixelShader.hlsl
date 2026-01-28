Texture2D SceneTex : register(t0);
SamplerState LinearClamp : register(s0);

cbuffer EdgeParams : register(b0)
{
    float2 TexelSize; // (1/width, 1/height)
    float Threshold; // 0.03Å`0.12 ñ⁄à¿
    float EdgePower; // 1Å`10 ñ⁄à¿
    float4 EdgeColor; // çïê¸Ç»ÇÁ(0,0,0,1)
};

float Luma(float3 c)
{
    return dot(c, float3(0.299, 0.587, 0.114));
}

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_TARGET
{
    float tl = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(-1, -1)).rgb);
    float t = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(0, -1)).rgb);
    float tr = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(1, -1)).rgb);

    float l = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(-1, 0)).rgb);
    float c = Luma(SceneTex.Sample(LinearClamp, uv).rgb);
    float r = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(1, 0)).rgb);

    float bl = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(-1, 1)).rgb);
    float b = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(0, 1)).rgb);
    float br = Luma(SceneTex.Sample(LinearClamp, uv + TexelSize * float2(1, 1)).rgb);

    float gx = (-1 * tl + 1 * tr) + (-2 * l + 2 * r) + (-1 * bl + 1 * br);
    float gy = (-1 * tl - 2 * t - 1 * tr) + (1 * bl + 2 * b + 1 * br);

    float edge = sqrt(gx * gx + gy * gy);
    edge = saturate((edge - Threshold) * EdgePower);

    float3 baseColor = SceneTex.Sample(LinearClamp, uv).rgb;
    float3 outColor = lerp(baseColor, EdgeColor.rgb, edge);

    return float4(outColor, 1.0);
}
