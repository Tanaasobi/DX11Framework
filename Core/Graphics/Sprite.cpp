//==============================================================================
// Sprite.cpp - スプライト描画クラス実装
//==============================================================================

#include "Sprite.h"
#include "Renderer.h"
#include "Core/System/Logger.h"
#include <d3dcompiler.h>

using namespace DirectX;

//==============================================================================
// 静的メンバ定義
//==============================================================================
ID3D11VertexShader* Sprite::m_VertexShader = nullptr;
ID3D11PixelShader* Sprite::m_PixelShader = nullptr;
ID3D11InputLayout* Sprite::m_InputLayout = nullptr;
ID3D11Buffer* Sprite::m_VertexBuffer = nullptr;
ID3D11Buffer* Sprite::m_ConstantBuffer = nullptr;

//==============================================================================
// スプライト用頂点構造体
//==============================================================================
struct VERTEX_SPRITE
{
	XMFLOAT3 Position;
	XMFLOAT2 TexCoord;
};

//==============================================================================
// 初期化
//==============================================================================
bool Sprite::Init()
{
	if (!CreateShaders()) return false;
	if (!CreateBuffers()) return false;

	Logger::Info("Sprite system initialized");
	return true;
}

//==============================================================================
// 終了
//==============================================================================
void Sprite::Uninit()
{
	SAFE_RELEASE(m_ConstantBuffer);
	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_InputLayout);
	SAFE_RELEASE(m_PixelShader);
	SAFE_RELEASE(m_VertexShader);

	Logger::Info("Sprite system uninitialized");
}

//==============================================================================
// シェーダー作成
//==============================================================================
bool Sprite::CreateShaders()
{
	HRESULT hr;
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// 頂点シェーダー
	hr = D3DCompileFromFile(
		L"Shader/SpriteVertexShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &vsBlob, &errorBlob
	);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("Sprite VS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return false;
	}

	// ピクセルシェーダー
	hr = D3DCompileFromFile(
		L"Shader/SpritePixelShader.hlsl",
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &psBlob, &errorBlob
	);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			Logger::ErrorFormat("Sprite PS error: %s", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		vsBlob->Release();
		return false;
	}

	// シェーダー作成
	ID3D11Device* device = Renderer::GetDevice();

	hr = device->CreateVertexShader(
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		nullptr, &m_VertexShader
	);
	if (FAILED(hr))
	{
		vsBlob->Release();
		psBlob->Release();
		return false;
	}

	hr = device->CreatePixelShader(
		psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
		nullptr, &m_PixelShader
	);
	if (FAILED(hr))
	{
		vsBlob->Release();
		psBlob->Release();
		return false;
	}

	// 入力レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = device->CreateInputLayout(
		layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		&m_InputLayout
	);

	vsBlob->Release();
	psBlob->Release();

	return SUCCEEDED(hr);
}

//==============================================================================
// バッファ作成
//==============================================================================
bool Sprite::CreateBuffers()
{
	ID3D11Device* device = Renderer::GetDevice();
	HRESULT hr;

	// 頂点バッファ（四角形）
	VERTEX_SPRITE vertices[] =
	{
		{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;

	hr = device->CreateBuffer(&vbDesc, &vbData, &m_VertexBuffer);
	if (FAILED(hr)) return false;

	// 定数バッファ
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(SPRITE_BUFFER);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = device->CreateBuffer(&cbDesc, nullptr, &m_ConstantBuffer);
	if (FAILED(hr)) return false;

	return true;
}

//==============================================================================
// 描画（シンプル版）
//==============================================================================
void Sprite::Draw(
	ID3D11ShaderResourceView* texture,
	float x, float y,
	float width, float height,
	float r, float g, float b, float a)
{
	DrawUV(texture, x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, r, g, b, a);
}

//==============================================================================
// 描画（UV指定あり）
//==============================================================================
void Sprite::DrawUV(
	ID3D11ShaderResourceView* texture,
	float x, float y,
	float width, float height,
	float u, float v,
	float uw, float vh,
	float r, float g, float b, float a)
{
	DrawTransform(texture, x, y, width, height, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, u, v, uw, vh, r, g, b, a);
}

//==============================================================================
// 描画（フル機能版）
//==============================================================================
void Sprite::DrawTransform(
	ID3D11ShaderResourceView* texture,
	float x, float y,
	float width, float height,
	float rotation,
	float scaleX, float scaleY,
	float pivotX, float pivotY,
	float u, float v,
	float uw, float vh,
	float r, float g, float b, float a)
{
	ID3D11DeviceContext* context = Renderer::GetDeviceContext();

	// ワールド行列を計算
	// 1. サイズを適用
	// 2. ピボット分オフセット（ピボットが回転・スケールの中心になる）
	// 3. 回転
	// 4. 位置に移動

	float offsetX = -pivotX * width;
	float offsetY = -pivotY * height;

	XMMATRIX scale = XMMatrixScaling(width * scaleX, height * scaleY, 1.0f);
	XMMATRIX pivotOffset = XMMatrixTranslation(-pivotX, -pivotY, 0.0f);
	XMMATRIX rot = XMMatrixRotationZ(rotation);
	XMMATRIX pivotRestore = XMMatrixTranslation(pivotX, pivotY, 0.0f);
	XMMATRIX translation = XMMatrixTranslation(x + offsetX, y + offsetY, 0.0f);

	// ピボットを中心に回転するための変換順序
	// スケール → ピボット位置へ移動 → 回転 → ピボット位置から戻す → 最終位置へ移動
	XMMATRIX world = scale * pivotOffset * rot * pivotRestore * translation;

	// 正射影行列（2D用）
	XMMATRIX view = XMMatrixIdentity();
	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(
		0.0f, static_cast<float>(SCREEN_WIDTH),
		static_cast<float>(SCREEN_HEIGHT), 0.0f,
		0.0f, 1.0f
	);

	// 定数バッファ更新
	SPRITE_BUFFER cb;
	cb.World = XMMatrixTranspose(world);
	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	cb.Color = XMFLOAT4(r, g, b, a);
	cb.UVOffset = XMFLOAT4(u, v, uw, vh);

	context->UpdateSubresource(m_ConstantBuffer, 0, nullptr, &cb, 0, 0);

	// パイプライン設定
	context->IASetInputLayout(m_InputLayout);
	context->VSSetShader(m_VertexShader, nullptr, 0);
	context->PSSetShader(m_PixelShader, nullptr, 0);
	context->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);

	// テクスチャ設定
	context->PSSetShaderResources(0, 1, &texture);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_SPRITE);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 深度無効・アルファブレンド有効で描画
	Renderer::SetDepthEnable(false);
	Renderer::SetBlendMode(BlendMode::Alpha);

	// 描画
	context->Draw(4, 0);

	// 深度を戻す
	Renderer::SetDepthEnable(true);
}
