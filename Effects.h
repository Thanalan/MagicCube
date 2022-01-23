//***************************************************************************************
// Effects.h by X_Jun(MKXJun) (C) 2018-2019 All Rights Reserved.
// Licensed under the MIT License.
//
// ������Ч������
// Simple effect management framework.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include <memory>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <d3d11_1.h>


class IEffect
{
public:
	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	IEffect() = default;

	// ��֧�ָ��ƹ���
	IEffect(const IEffect&) = delete;
	IEffect& operator=(const IEffect&) = delete;

	// ����ת��
	IEffect(IEffect&& moveFrom) = default;
	IEffect& operator=(IEffect&& moveFrom) = default;

	virtual ~IEffect() = default;

	// ���²��󶨳���������
	virtual void Apply(ComPtr<ID3D11DeviceContext> deviceContext) = 0;
};


class BasicEffect : public IEffect
{
public:

	BasicEffect();
	virtual ~BasicEffect() override;

	BasicEffect(BasicEffect&& moveFrom);
	BasicEffect& operator=(BasicEffect&& moveFrom);

	// ��ȡ����
	static BasicEffect& Get();

	

	// ��ʼ��Basic.hlsli������Դ����ʼ����Ⱦ״̬
	bool InitAll(ComPtr<ID3D11Device> device);


	//
	// ��Ⱦģʽ�ı��
	//

	// Ĭ��״̬������
	void SetRenderDefault(ComPtr<ID3D11DeviceContext> deviceContext);


	

	//
	// ��������
	//

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);
	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX W, DirectX::CXMMATRIX V, DirectX::CXMMATRIX P);

	
	//
	// �����������
	//

	void SetTextureArray(ComPtr<ID3D11ShaderResourceView> textureArray);
	void SetTexIndex(int index);

	// Ӧ�ó�����������������Դ�ı��
	void Apply(ComPtr<ID3D11DeviceContext> deviceContext);
	
private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};










#endif