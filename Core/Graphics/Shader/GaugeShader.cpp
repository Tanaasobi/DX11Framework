#include "GaugeShader.h"
#include "Core/Graphics//Renderer.h"

//==============================================================================
// 入力レイアウト作成
//==============================================================================
bool GaugeShader::CreateInputLayout(ID3DBlob* vsBlob)
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT hr = Renderer::GetDevice()->CreateInputLayout(
		layout, ARRAYSIZE(layout),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		&m_InputLayout);

	return SUCCEEDED(hr);
}
