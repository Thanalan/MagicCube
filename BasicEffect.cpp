#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// ��������Effects.h��d3dUtil.h����
#include "Vertex.h"
using namespace DirectX;

// ��ɫ���ֽ��룬��������
#include "HLSL/Basic_VS.inc"
#include "HLSL/Basic_PS.inc"


//
// BasicEffect::Impl ��Ҫ����BasicEffect�Ķ���
//

class BasicEffect::Impl : public AlignedType<BasicEffect::Impl>
{
public:

	//
	// ��Щ�ṹ���ӦHLSL�Ľṹ�塣��Ҫ��16�ֽڶ���
	//

	struct CBChangesEveryDrawing
	{
		int texIndex;
		XMFLOAT3 gPad;
	};

	struct CBChangesEveryCube
	{
		XMMATRIX world;
	};

	struct CBChangesEveryFrame
	{
		XMMATRIX view;
	};

	struct CBChangesOnResize
	{
		XMMATRIX proj;
	};

public:
	// ������ʽָ��
	Impl() = default;
	~Impl() = default;

public:
	// ��Ҫ16�ֽڶ�������ȷ���ǰ��
	CBufferObject<0, CBChangesEveryDrawing> cbDrawing;		// ÿ�ζ�����Ƶĳ���������
	CBufferObject<1, CBChangesEveryCube>    cbCube;			// ÿ����������Ƶĳ���������
	CBufferObject<2, CBChangesEveryFrame>   cbFrame;		// ÿ֡���Ƶĳ���������
	CBufferObject<3, CBChangesOnResize>     cbOnResize;		// ÿ�δ��ڴ�С����ĳ���������
	BOOL isDirty;											// �Ƿ���ֵ���
	std::vector<CBufferBase*> cBufferPtrs;					// ͳһ�����������еĳ���������


	ComPtr<ID3D11VertexShader> basicVS;						// ������ɫ��
	ComPtr<ID3D11PixelShader>  basicPS;						// ������ɫ��

	ComPtr<ID3D11SamplerState> ssLinearWrap;				// ���Բ�����״̬

	ComPtr<ID3D11InputLayout>  vertexLayout;				// �������벼��

	ComPtr<ID3D11ShaderResourceView> textureArray;			// ���ڻ��Ƶ���������

};

//
// BasicEffect
//

namespace
{
	// BasicEffect����
	static BasicEffect * pInstance = nullptr;
}

BasicEffect::BasicEffect()
{
	if (pInstance)
		throw std::exception("BasicEffect is a singleton!");
	pInstance = this;
	pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{
}

BasicEffect::BasicEffect(BasicEffect && moveFrom)
{
	pImpl.swap(moveFrom.pImpl);
}

BasicEffect & BasicEffect::operator=(BasicEffect && moveFrom)
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

BasicEffect & BasicEffect::Get()
{
	if (!pInstance)
		throw std::exception("BasicEffect needs an instance!");
	return *pInstance;
}


bool BasicEffect::InitAll(ComPtr<ID3D11Device> device)
{
	if (!device)
		return false;

	if (!pImpl->cBufferPtrs.empty())
		return true;

	// ����������ɫ��
	HR(device->CreateVertexShader(g_Basic_VS, sizeof(g_Basic_VS), nullptr, pImpl->basicVS.GetAddressOf()));
	// �������㲼��
	HR(device->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
		g_Basic_VS, sizeof(g_Basic_VS), pImpl->vertexLayout.GetAddressOf()));

	// ����������ɫ��
	HR(device->CreatePixelShader(g_Basic_PS, sizeof(g_Basic_PS), nullptr, pImpl->basicPS.GetAddressOf()));

	// ����������״̬
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof sd);
	sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.Filter = D3D11_FILTER_ANISOTROPIC;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	HR(device->CreateSamplerState(&sd, pImpl->ssLinearWrap.GetAddressOf()));

	pImpl->cBufferPtrs.assign({
		&pImpl->cbDrawing, 
		&pImpl->cbCube,
		&pImpl->cbFrame, 
		&pImpl->cbOnResize});

	// ��������������
	for (auto& pBuffer : pImpl->cBufferPtrs)
	{
		pBuffer->CreateBuffer(device);
	}

	return true;
}

void BasicEffect::SetRenderDefault(ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(pImpl->vertexLayout.Get());
	deviceContext->VSSetShader(pImpl->basicVS.Get(), nullptr, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->PSSetShader(pImpl->basicPS.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, pImpl->ssLinearWrap.GetAddressOf());
	deviceContext->OMSetDepthStencilState(nullptr, 0);
	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void XM_CALLCONV BasicEffect::SetWorldMatrix(DirectX::FXMMATRIX W)
{
	auto& cBuffer = pImpl->cbCube;
	cBuffer.data.world = XMMatrixTranspose(W);
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetViewMatrix(FXMMATRIX V)
{
	auto& cBuffer = pImpl->cbFrame;
	cBuffer.data.view = XMMatrixTranspose(V);
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetProjMatrix(FXMMATRIX P)
{
	auto& cBuffer = pImpl->cbOnResize;
	cBuffer.data.proj = XMMatrixTranspose(P);
	pImpl->isDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetWorldViewProjMatrix(FXMMATRIX W, CXMMATRIX V, CXMMATRIX P)
{
	pImpl->cbCube.data.world = XMMatrixTranspose(W);
	pImpl->cbFrame.data.view = XMMatrixTranspose(V);
	pImpl->cbOnResize.data.proj = XMMatrixTranspose(P);

	auto& pCBuffers = pImpl->cBufferPtrs;
	pCBuffers[1]->isDirty = pCBuffers[2]->isDirty = pCBuffers[3]->isDirty = true;
	pImpl->isDirty = true;
}

void BasicEffect::SetTextureArray(ComPtr<ID3D11ShaderResourceView> textureArray)
{
	pImpl->textureArray = textureArray;
}

void BasicEffect::SetTexIndex(int index)
{
	auto& cBuffer = pImpl->cbDrawing;
	cBuffer.data.texIndex = index;
	pImpl->isDirty = cBuffer.isDirty = true;
}

void BasicEffect::Apply(ComPtr<ID3D11DeviceContext> deviceContext)
{
	auto& pCBuffers = pImpl->cBufferPtrs;
	// ���������󶨵���Ⱦ������
	pCBuffers[1]->BindVS(deviceContext);
	pCBuffers[2]->BindVS(deviceContext);
	pCBuffers[3]->BindVS(deviceContext);

	pCBuffers[0]->BindPS(deviceContext);

	// ��������
	deviceContext->PSSetShaderResources(0, 1, pImpl->textureArray.GetAddressOf());

	if (pImpl->isDirty)
	{
		pImpl->isDirty = false;
		for (auto& pCBuffer : pCBuffers)
		{
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}




