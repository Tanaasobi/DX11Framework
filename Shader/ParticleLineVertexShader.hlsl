//==============================================================================
// ParticleLineVertexShader.hlsl - ラインパーティクル用頂点シェーダー
//==============================================================================



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
    // 頂点データ（slot 0）
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    
    // インスタンスデータ（slot 1）
    float3 InstPosition : INST_POSITION;
    float InstSize : INST_SIZE;
    float4 InstColor : INST_COLOR;
    float InstRotation : INST_ROTATION;
    float3 InstVelocity : INST_VELOCITY;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float4 Color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // 形状変形 (速度に応じて伸ばす)
    float3 localPos = input.Position;
    float speed = length(input.InstVelocity);
    float stretchFactor = max(1.0f, speed * 0.25f);

    // 速度に応じてシフト (伸びる方向に移動させる)
    float shift = input.InstSize * (stretchFactor - 1.0f);
    
    localPos.x *= input.InstSize * 1.0f * stretchFactor; // 進行方向
    localPos.y *= input.InstSize * 0.5f; // 幅
    localPos.z = 0.0f;

    // 伸びる方向にシフト
    localPos.x += shift;

    // 回転行列作成 (速度軸ビルボード)
    float3 velocity = input.InstVelocity;
    float3 forward;

    if (length(velocity) < 0.001f)
    {
        forward = float3(1, 0, 0);
    }
    else
    {
        forward = normalize(velocity);
    }
    
    // カメラ方向 (View行列から取得)
    float3 camDir = normalize(float3(View._13, View._23, View._33));
    
    // 右ベクトル
    float3 right = cross(forward, camDir);
    if (length(right) < 0.001f)
    {
        right = float3(0, 1, 0);
    }
    else
    {
        right = normalize(right);
    }
    
    // 上ベクトル
    float3 up = cross(right, forward);
    up = normalize(up);

    float3x3 rotMat;
    rotMat[0] = forward;
    rotMat[1] = right;
    rotMat[2] = up;

    // 座標変換
    float3 rotatedPos = mul(localPos, rotMat);
    float3 worldPos = rotatedPos + input.InstPosition;

    output.Position = mul(float4(worldPos, 1.0f), mul(View, Projection));
    output.Color = input.InstColor;
    output.TexCoord = input.TexCoord;
    
    return output;
}
