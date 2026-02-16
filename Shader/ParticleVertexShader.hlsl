//==============================================================================
// ParticleVertexShader.hlsl - パーティクル用頂点シェーダー（ビルボード）
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

    // 回転（Z軸）
    float rad = input.InstRotation * (3.14159265f / 180.0f);
    float cosR = cos(rad);
    float sinR = sin(rad);

    float2 rotatedPos;
    rotatedPos.x = input.Position.x * cosR - input.Position.y * sinR;
    rotatedPos.y = input.Position.x * sinR + input.Position.y * cosR;

    // スケール適用
    rotatedPos *= input.InstSize;

    // ビルボード：View行列の逆回転を適用
    // View行列の上3x3を転置して右方向・上方向を取得
    float3 right = float3(View[0][0], View[1][0], View[2][0]);
    float3 up = float3(View[0][1], View[1][1], View[2][1]);

    // ワールド位置計算
    float3 worldPos = input.InstPosition;
    worldPos += right * rotatedPos.x;
    worldPos += up * rotatedPos.y;

    // View・Projection変換
    float4 viewPos = mul(float4(worldPos, 1.0f), View);
    output.Position = mul(viewPos, Projection);

    output.TexCoord = input.TexCoord;
    output.Color = input.InstColor;

    return output;
}
